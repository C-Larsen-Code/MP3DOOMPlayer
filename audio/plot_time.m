% Clear command window & workspace, and close all figures
clc, clear, close all;

% Select audio files to analyze
[fname,location] = uigetfile(...
    '*.aifc;*.aiff;*.aif;*.au;*.flac;*.ogg;*.opus;*.mp3;*.m4a;*.mp4;*.wav',...
    'Select one or more audio files',...
    'MultiSelect','on');
if isequal(fname,0) % user canceled selection
    disp('No file(s) selected');
    return;
elseif ischar(fname) % convert to cell array if single file selected
    fname = {fname};
end

% Plot audio data
for i = 1:length(fname)
    % read audio wave file into a matrix
    % returns: [data, sample frequency]
    [x, fs] = audioread(fullfile(location,fname{i}));
    figure;
    plot(x);
    title(fname{i});
    xlabel('Time (sample)');
    ylabel('Amplitude');
end
