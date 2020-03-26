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
        samplesPerCode = pow2(10);
        
        % Read data
        numOfDiffs = 100;
        data1  = fread(fid1, dataAdaptCoeff*(numOfDiffs+1)*samplesPerCode, settings.dataType)';
        data2  = fread(fid2, dataAdaptCoeff*(numOfDiffs+1)*samplesPerCode, settings.dataType)';
    end
    
    % Get differences
    diffs = zeros(settings.numberOfChannels,numOfDiffs);
    h = waitbar(0,'Getting differences...');
    for l = 1:settings.numberOfChannels
        for k = 0:(numOfDiffs - 1)
            diffs(l,k+1) = acquisition3(data1, data2, samplesPerCode, 1+k*samplesPerCode, settings);
            waitbar((k + numOfDiffs*(l-1))/(settings.numberOfChannels*numOfDiffs),h)
        end;
    end;
    close(h);
    diffs = mean(diffs,2);
    
    % Get etalones and errors
    az = 0;
    el = 0;
    wl = 300/1575.42;
    L = 0.844 - 0.05;
    errors = zeros(360,21); % ---------------------------------------------
    for i = 0:20
        for t = 1:360
            errors(t,i+1) = my_error(rad2deg(2 * pi * ph_rot(az + (t-1),el,(L + i*0.005)) / wl),diffs);
        end;
    end;
    [~,dL] = min(min(errors));
    L = L + (dL-1)*0.005;
    errors = zeros(360,1); % ----------------------------------------------
    phases = zeros(360,1);
    for t = 1:360
        phases(t) = rad2deg(2 * pi * ph_rot(az + (t-1),el,L) / wl);
        errors(t) = my_error(phases(t),diffs);
    end;
    %[ymin,tmin] = min(errors);
    %phases = diff_norm(rad2deg(2 * pi * ph_rot(az + (tmin-1),el,L) / wl));
    %save('res','errors');
    
    figure;
    plot(errors);
    disp('Post processing of the signal is over.');

else
    % Error while opening the data file.
    error('Unable to read file %s: %s.', settings.fileName, message);
end % if (fid > 0)