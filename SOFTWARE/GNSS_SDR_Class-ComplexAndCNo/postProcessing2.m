%% Initialization =========================================================
disp ('Starting processing...');

[fid1, message1] = fopen(settings.fileName1, 'rb');
[fid2, message2] = fopen(settings.fileName2, 'rb');

%Initialize the multiplier to adjust for the data type
if (settings.fileType==1) 
    dataAdaptCoeff=1;
else
    dataAdaptCoeff=2;
end

%If success, then process the data
if (fid1 > 0 && fid2 > 0)
    
    % Move the starting point of processing. Can be used to start the
    % signal processing at any point in the data record (e.g. good for long
    % records or for signal processing in blocks).
    fseek(fid1, dataAdaptCoeff*settings.skipNumberOfBytes, 'bof');
    fseek(fid2, dataAdaptCoeff*settings.skipNumberOfBytes, 'bof');

%% Acquisition ============================================================

    % Do acquisition if it is not disabled in settings or if the variable
    % acqResults does not exist.
    if ((settings.skipAcquisition == 0) || ~exist('acqResults', 'var'))
        
        % Find number of samples per spreading code
        samplesPerCode = round(settings.samplingFreq / ...
                           (settings.codeFreqBasis / settings.codeLength));
        
        % Read data for acquisition
        numOfDiffs = 1000;
        data1  = fread(fid1, dataAdaptCoeff*(numOfDiffs+1)*samplesPerCode, settings.dataType)';
        data2  = fread(fid2, dataAdaptCoeff*(numOfDiffs+1)*samplesPerCode, settings.dataType)';
    
        %--- Do the acquisition -------------------------------------------
        disp ('   Acquiring first set of satellites...');
        acqResults = acquisition(data1(1:(11*samplesPerCode)), settings);
        plotAcquisition2(acqResults);
        
        disp ('   Acquiring second set of satellites...');
        acqResults2 = acquisition(data2(1:(11*samplesPerCode)), settings);
        plotAcquisition2(acqResults2);
    end
    
    % Get the number of common satelites
    settings.numberOfChannels = sum((acqResults.carrFreq ~= 0) .* (acqResults2.carrFreq ~= 0));

%% Initialize channels and prepare for the run ============================

    % Start further processing only if a GNSS signal was acquired (the
    % field FREQUENCY will be set to 0 for all not acquired signals)
    if (any(acqResults.carrFreq))
        channel = preRun2(acqResults, acqResults2, 1, settings);
        showChannelStatus(channel, settings);
    else
        % No satellites to track, exit
        disp('No GNSS signals detected, signal processing finished.');
        trackResults = [];
        return;
    end
    
    % Get differences
    sat = [channel.PRN];
    diffs = zeros(settings.numberOfChannels,numOfDiffs);
    h = waitbar(0,'Getting differences...');
    aaa = zeros(settings.numberOfChannels,numOfDiffs);
    for l = 1:settings.numberOfChannels
        for k = 0:(numOfDiffs - 1)
            [diffs(l,k+1), aaa(l,k+1)] = acquisition2(data1, data2, sat(l), 1+k*samplesPerCode, settings);
            waitbar((k + numOfDiffs*(l-1))/(settings.numberOfChannels*numOfDiffs),h);
        end;
    end;
    close(h);
    diffs = mean(diffs,2);
    aaa = mean(aaa,2);

%% Track the signal =======================================================
    startTime = now;
    disp(['   Tracking started at ', datestr(startTime)]);

    % Process all channels for given data block
    [trackResults, channel] = tracking(fid1, channel, settings);

    % Close the data file
    fclose(fid1);
    fclose(fid2);
    
    disp(['   Tracking is over (elapsed time ', ...
                                        datestr(now - startTime, 13), ')'])             

%% Calculate navigation solutions =========================================
    disp('   Calculating navigation solutions...');
    navSolutions = postNavigation(trackResults, settings);

    disp('   Processing is complete for this data block');
    
    % Get etalones and errors
    %az = mean(navSolutions.channel.az,2);
    %el = mean(navSolutions.channel.el,2);
    az = [75.5 278.2 44.8 0 0 298.1 0 0 144.3 222.8 98.7 326.9 0 0 0 0 350.8 0 0 47.2 0 0 106.0 0 0 0 0 201.9 0 0 0 43.1];
    el = [19.0   8.6 38.2 0 0  43.4 0 0  29.1  34.6  7.0   5.0 0 0 0 0  83.8 0 0 49.4 0 0  35.6 0 0 0 0  27.9 0 0 0  2.2];
    az = az(sat);
    el = el(sat);
    wl = settings.c/1575.42e6;
    L = 0.844;
    if (0 > 1)
        errors = zeros(360,360); % ----------------------------------------
        for i = 1:360
            for t = 1:360
                errors(t,i) = my_error(rad2deg(2 * pi * ph_rot(az + (t-1),el,L) / wl),diffs + i);
            end;
        end;
        [~,df] = min(min(errors));
        diffs = diffs + df;
        errors = zeros(360,21); % -----------------------------------------
        L = 0.844 - 0.05;
        for i = 0:20
            for t = 1:360
                errors(t,i+1) = my_error(rad2deg(2 * pi * ph_rot(az + (t-1),el,(L + i*0.005)) / wl),diffs);
            end;
        end;
        [~,dL] = min(min(errors));
        L = L + (dL-1)*0.005;
    end
    errors = zeros(360,1); % ----------------------------------------------
    for t = 1:360
        errors(t) = my_error(rad2deg(2 * pi * ph_rot(az + (t-1),el,L) / wl),diffs);
    end;
    [ymin,tmin] = min(errors);
    v1 = [-3813409.771,3554349.703];
    v2 = [-3813477.954,3554276.552];
    v3 = [-3813467.954,3554287.281];
    v = v1 - v3;
    true1 = rad2deg(acos(v(1)/(norm(v,2))));
    true2 = rad2deg(acos(v(2)/(norm(v,2))));
    ans1 = abs(true1 - tmin);
    ans2 = abs(true2 - tmin);
    genDiffs = 28.40106584 - 29.10454435;
    %phases = diff_norm(rad2deg(2 * pi * ph_rot(az + (tmin-1),el,L) / wl));
    %save('res','diffs','phases','errors','df','dL');
    
%% Plot all results =======================================================
    disp ('   Ploting results...');
    if settings.plotTracking
        plotTracking2(1:settings.numberOfChannels, trackResults, settings);
    end
    plotNavigation2(navSolutions, settings);

    disp('Post processing of the signal is over.');

else
    % Error while opening the data file.
    error('Unable to read file %s: %s.', settings.fileName, message);
end % if (fid > 0)