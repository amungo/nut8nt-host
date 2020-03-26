function [channel] = preRun2(acqResults, acqResults2, awd, settings)
%Function initializes tracking channels from acquisition data. The acquired
%signals are sorted according to the signal strength. This function can be
%modified to use other satellite selection algorithms or to introduce
%acquired signal properties offsets for testing purposes.
%
%[channel] = preRun(acqResults, settings)
%
%   Inputs:
%       acqResults  - results from acquisition.
%       settings    - receiver settings
%
%   Outputs:
%       channel     - structure contains information for each channel (like
%                   properties of the tracked signal, channel status etc.). 

%--------------------------------------------------------------------------
%                           SoftGNSS v3.0
% 
% Copyright (C) Darius Plausinaitis
% Written by Darius Plausinaitis
% Based on Peter Rinder and Nicolaj Bertelsen
%--------------------------------------------------------------------------
%This program is free software; you can redistribute it and/or
%modify it under the terms of the GNU General Public License
%as published by the Free Software Foundation; either version 2
%of the License, or (at your option) any later version.
%
%This program is distributed in the hope that it will be useful,
%but WITHOUT ANY WARRANTY; without even the implied warranty of
%MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
%GNU General Public License for more details.
%
%You should have received a copy of the GNU General Public License
%along with this program; if not, write to the Free Software
%Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
%USA.
%--------------------------------------------------------------------------

%CVS record:
%$Id: preRun.m,v 1.8.2.20 2006/08/14 11:38:22 dpl Exp $

%% Initialize all channels ================================================
channel                 = [];   % Clear, create the structure

channel.PRN             = 0;    % PRN number of the tracked satellite
channel.acquiredFreq    = 0;    % Used as the center frequency of the NCO
channel.codePhase       = 0;    % Position of the C/A  start

channel.status          = '-';  % Mode/status of the tracking channel
                                % "-" - "off" - no signal to track
                                % "T" - Tracking state

%--- Copy initial data to all channels ------------------------------------
channel = repmat(channel, 1, settings.numberOfChannels);

%% Copy acquisition results ===============================================

%--- Sort peaks to find strongest signals, keep the peak index information
comSatInd = ((acqResults.carrFreq ~= 0) .* (acqResults2.carrFreq ~= 0)) .* (1:32);
comSatInd(comSatInd == 0) = [];
[~, PRNindexes1]          = sort(acqResults.peakMetric, 2, 'descend');
[~, PRNindexes2]          = sort(acqResults2.peakMetric, 2, 'descend');
for i = 1:32
    if ~ismember(PRNindexes1(i),comSatInd)
        PRNindexes1(i) = 0;
    end
    if ~ismember(PRNindexes2(i),comSatInd)
        PRNindexes2(i) = 0;
    end
end;
PRNindexes1(PRNindexes1 == 0) = [];
PRNindexes2(PRNindexes2 == 0) = [];
if awd
    PRNindexes = [];
    for i = 0:(length(PRNindexes1)-3)
        PRNindexes = union(PRNindexes, intersect(PRNindexes1((1:3) + i),PRNindexes2((1:3) + i),'stable'),'stable');
    end
    if (length(PRNindexes) < settings.numberOfChannels)
        PRNindexes = union(PRNindexes, setdiff(PRNindexes1,PRNindexes,'stable'),'stable');
    end;
else
    PRNindexes = PRNindexes1;
end;

%--- Load information about each satellite --------------------------------
% Maximum number of initialized channels is number of detected signals, but
% not more as the number of channels specified in the settings.
for ii = 1:settings.numberOfChannels
    channel(ii).PRN          = PRNindexes(ii);
    channel(ii).acquiredFreq = acqResults.carrFreq(PRNindexes(ii));
    channel(ii).codePhase    = acqResults.codePhase(PRNindexes(ii));
    
    % Set tracking into mode (there can be more modes if needed e.g. pull-in)
    channel(ii).status       = 'T';
end
