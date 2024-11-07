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

% Plot audio power spectrum
for i = 1:length(fname)
    % read audio wave file into a matrix
    % returns: [data, sample frequency]
    [x, fs] = audioread(fullfile(location,fname{i}));
    t = 0:1/fs:(length(x)-1)/fs;
    m = length(x);    % original sample length
    n = pow2(nextpow2(m)); % transform length
    y = fft(x,n);     % DFT of signal
    f = (0:n-1)*(fs/n);
    power = abs(y).^2/n;
    figure;
    plot(f(1:floor(n/2)),power(1:floor(n/2)));
    title(fname{i});
    xlabel('Frequency');
    ylabel('Power');
end
