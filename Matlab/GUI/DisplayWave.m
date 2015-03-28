function [ ] = DisplayWave( data, handles )
%DISPLAYWAVE Summary of this function goes here
%   Detailed explanation goes here

time = linspace(0,10,length(data));
plot(time,data,'LineWidth',2);

% Display trigger lines
line([handles.trigger_position handles.trigger_position], [-10 270],'LineStyle',':','LineWidth',2,'Color','k');
line([0 10], [handles.trigger_level handles.trigger_level],'LineStyle',':','LineWidth',2,'Color','k');
    
% Display resolution
text(8, 260, strcat('\bf',handles.strings(handles.adc_mode+1,:),'/div'), 'FontSize',16);

ylim([-10 270]);
grid on;
set(gca,'XTick',0:1:10);
drawnow;

end

