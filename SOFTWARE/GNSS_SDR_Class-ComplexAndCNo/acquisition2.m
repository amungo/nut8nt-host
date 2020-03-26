function [res, awd] = acquisition2(signal1, signal2, prn, start, settings)
%function [res, codePhase, frequencyBinIndex, peak] = acquisition2(signal1, signal2, prn, start, settings)
%% Initialization =========================================================

% Find number of samples per spreading code
samplesPerCode = round(settings.samplingFreq / ...
    (settings.codeFreqBasis / settings.codeLength));

% Find sampling period
ts = 1 / settings.samplingFreq;

% Find phase points of the local carrier wave
phasePoints = (0 : (samplesPerCode-1)) * 2 * pi * ts;

% Number of the frequency bins for the given acquisition band (500Hz steps)
numberOfFrqBins = round(settings.acqSearchBand * 2) + 1;

% Generate all C/A codes and sample them according to the sampling freq.
caCodesTable = makeCaTable(settings);

%--- Initialize arrays to speed up the code -------------------------------
% Search results of all frequency bins and code shifts (for one satellite)
results     = zeros(numberOfFrqBins, samplesPerCode);
results2    = zeros(numberOfFrqBins, samplesPerCode);

results3    = zeros(numberOfFrqBins, samplesPerCode);
results4    = zeros(numberOfFrqBins, samplesPerCode);

% Carrier frequencies of the frequency bins
frqBins     = zeros(1, numberOfFrqBins);

%% Correlate signals ======================================================
%--- Perform DFT of C/A code ------------------------------------------
caCodeFreqDom = conj(fft(caCodesTable(prn,:)));

%--- Make the correlation for whole frequency band (for all freq. bins)
for frqBinIndex = 1:numberOfFrqBins
    
    %--- Generate carrier wave frequency grid (0.5kHz step) -----------
    frqBins(frqBinIndex) = settings.IF - ...
        (settings.acqSearchBand/2) * 1000 + ...
        0.5e3 * (frqBinIndex - 1);
    
    %--- Generate local sine and cosine -------------------------------
    sigCarr = exp(complex(0,frqBins(frqBinIndex) * phasePoints));
    
    %--- "Remove carrier" from the signal -----------------------------
    I1      = real(sigCarr .* signal1(start:(start+samplesPerCode-1)));
    Q1      = imag(sigCarr .* signal1(start:(start+samplesPerCode-1)));
    
    I2      = real(sigCarr .* signal2(start:(start+samplesPerCode-1)));
    Q2      = imag(sigCarr .* signal2(start:(start+samplesPerCode-1)));
    
    results3(frqBinIndex, :) = complex(I1,Q1);
    results4(frqBinIndex, :) = complex(I2,Q2);
    
    IC      = real(sigCarr .* signal1((start+samplesPerCode):(start+(2*samplesPerCode)-1)));
    QC      = imag(sigCarr .* signal1((start+samplesPerCode):(start+(2*samplesPerCode)-1)));
    
    ID      = real(sigCarr .* signal2((start+samplesPerCode):(start+(2*samplesPerCode)-1)));
    QD      = imag(sigCarr .* signal2((start+samplesPerCode):(start+(2*samplesPerCode)-1)));
    
    %--- Convert the baseband signal to frequency domain --------------
    IQfreqDom1 = fft(complex(I1,Q1));
    IQfreqDom2 = fft(complex(I2,Q2));
    
    IQfreqDomC = fft(complex(IC,QC));
    IQfreqDomD = fft(complex(ID,QD));
    
    %--- Multiplication in the frequency domain (correlation in time
    %domain)
    convCodeIQ1 = IQfreqDom1 .* caCodeFreqDom;
    convCodeIQ2 = IQfreqDom2 .* caCodeFreqDom;
    
    convCodeIQC = IQfreqDomC .* caCodeFreqDom;
    convCodeIQD = IQfreqDomD .* caCodeFreqDom;
    
    %--- Perform inverse DFT and store correlation results ------------
    acqRes1 = ifft(convCodeIQ1);
    acqRes2 = ifft(convCodeIQ2);
    
    acqRes3 = ifft(convCodeIQC);
    acqRes4 = ifft(convCodeIQD);
    
    %--- Check which msec had the greater power and save that, will
    %"blend" 1st and 2nd msec but will correct data bit issues
    if (max(abs(acqRes1)) > max(abs(acqRes3)))
        results(frqBinIndex, :) = acqRes1;
        results2(frqBinIndex, :) = acqRes2;
    else
        results(frqBinIndex, :) = acqRes3;
        results2(frqBinIndex, :) = acqRes4;
    end
    
end % frqBinIndex = 1:numberOfFrqBins

%--- Find the correlation peak and the carrier frequency --------------
[~, frequencyBinIndex] = max(max(results, [], 2));

%--- Find code phase of the same correlation peak ---------------------
[~, codePhase] = max(max(results));

res = rad2deg(angle(results(frequencyBinIndex, codePhase) * conj(results2(frequencyBinIndex, codePhase))));

%[~, i1] = max(max(results3, [], 2));
%[~, i2] = max(max(results3));
%awd = rad2deg(angle(results3(i1, i2) * conj(results4(i1, i2))));
awd = rad2deg(angle(median(median(results3)) * conj(median(median(results4)))));

% %--- Find 1 chip wide C/A code phase exclude range around the peak ----
% samplesPerCodeChip   = round(settings.samplingFreq / settings.codeFreqBasis);
% excludeRangeIndex1 = codePhase - samplesPerCodeChip;
% excludeRangeIndex2 = codePhase + samplesPerCodeChip;
% 
% %--- Correct C/A code phase exclude range if the range includes array
% %boundaries
% if excludeRangeIndex1 < 2
%     codePhaseRange = excludeRangeIndex2 : ...
%         (samplesPerCode + excludeRangeIndex1);
%     
% elseif excludeRangeIndex2 >= samplesPerCode
%     codePhaseRange = (excludeRangeIndex2 - samplesPerCode) : ...
%         excludeRangeIndex1;
% else
%     codePhaseRange = [1:excludeRangeIndex1, ...
%         excludeRangeIndex2 : samplesPerCode];
% end
% 
% %--- Find the second highest correlation peak in the same freq. bin ---
% secondPeakSize = max(results(frequencyBinIndex, codePhaseRange));
% 
% %--- Store result -----------------------------------------------------
% peak = peakSize/secondPeakSize;
end