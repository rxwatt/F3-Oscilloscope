clear all;

% Text constatnts
Strings = ['500 ns'; '  1 us'; '  2 us'; '  5 us'; ' 10 us'; ' 20 us';...
    ' 50 us'; '100 us'; '200 us'; '500 us'; '  1 ms'; '  2 ms'; '  5 ms';...
    ' 10 ms'; ' 20 ms'; ' 50 ms'; '100 ms'; '200 ms'; '500 ms'];
ADC_Scan_Data = [];

% Variables and defaults
try
    load('Settings');
catch err
    resolution = 16;
    trigger_channel = 2;
    trigger_level = 120;
    trigger_type = 0;
    trigger_position = 5;
end;
ADC_Scan_Data = [];
scan_cnt = 0;

% Override settings
%resolution = 16;


% Figure initialization
fig = figure;
key = [];
set(fig,'keypress','code=double(get(fig,''currentchar'')); key=get(fig,''currentchar'');');

% Com-port initialization
com_port = '/dev/tty.usbmodemfd131';
%com_port = 'com8';
com = serial(com_port,'BaudRate',115200,'InputBufferSize',32768,'Timeout',5);
fopen(com);
TX_Settings;

while (1)
    
    % Receive data
    fwrite(com,'$O>');
    ADC_Header = fread(com,4,'uint8');
    adc_mode = ADC_Header(1);
    adc_sample_length = ADC_Header(2)*256 + ADC_Header(3);
    adc_triggered = ADC_Header(4);
    ADC_Buffer = fread(com,adc_sample_length*4,'uint8');
    scan_cnt = scan_cnt+1;
    
    % Sort data
    ADC_Data = zeros(adc_sample_length,4);
    indexes = 1:2:adc_sample_length*2;
    ADC_Data(:,1) = ADC_Buffer(indexes);
    ADC_Data(:,2) = ADC_Buffer(indexes+1);
    ADC_Data(:,3) = ADC_Buffer(indexes+adc_sample_length*2);
    ADC_Data(:,4) = ADC_Buffer(indexes+1+adc_sample_length*2);
    
    % Append scan data
    
    
    if (resolution >= 16)       
        switch resolution
            case 18
                frames_per_screen = 100;
            case 17
                frames_per_screen = 40;
            case 16
                frames_per_screen = 20;
            otherwise
                frames_per_screen = 1;
        end;
        if (scan_cnt <= frames_per_screen)
            ADC_Scan_Data = [ADC_Scan_Data; ADC_Data]; %#ok<AGROW>
            ADC_Data = ADC_Scan_Data;
        else
            ADC_Scan_Data = [];
            scan_cnt = 0;
        end;
        time = linspace(0,scan_cnt/frames_per_screen*10,length(ADC_Data));
    else
        time = linspace(0,10,length(ADC_Data));
    end;
    
    % Display data
    plot(time,ADC_Data,'LineWidth',2);
    
    % Display trigger lines
    line([trigger_position trigger_position], [-10 270],'LineStyle',':','LineWidth',2,'Color','k');
    line([0 10], [trigger_level trigger_level],'LineStyle',':','LineWidth',2,'Color','k');
    
    % Display resolution
    text(8, 260, strcat('\bf',Strings(adc_mode+1,:),'/div'), 'FontSize',16);
    
    % Display trigger status
    switch trigger_channel
        case 1
            ch_color = 'b';
        case 2
            ch_color = [0 0.5 0];
        case 3
            ch_color = 'r';
        case 4
            ch_color = [0 0.7 0.7];
        otherwise
            ch_color = 'k';
    end;
    text(1, 260, strcat('\bfCH',int2str(trigger_channel)), 'FontSize',16,'Color', ch_color);
    if (adc_triggered)
        text(3, 260, '\bfTrig!', 'FontSize',16);
    else
        text(3, 260, '\bfTrig?', 'FontSize',16);
    end;
    
    % Draw image
    ylim([-10 270]);
    grid on;
    set(gca,'XTick',0:1:10);
    drawnow;
    
    % Scan key pressed
    if (~isempty(key))
        switch key
            case 44 % [,<]
                if (trigger_position > 0)
                    trigger_position = trigger_position - 1;
                end;
            case 46 % [.>]
                if (trigger_position < 10)
                    trigger_position = trigger_position + 1;
                end;
            case 28 % [Left]
                if (resolution < (size(Strings,1)-1))
                    resolution = resolution + 1;
                    if (resolution > 15)
                        ADC_Scan_Data = [];
                        scan_cnt = 0;
                    end;
                end;
            case 29 % [Right]
                if (resolution > 0)
                    resolution = resolution - 1;
                    if (resolution >= 16)
                        ADC_Scan_Data = [];
                        ADC_Data = [];
                        scan_cnt = 0;
                    end;
                end;
            case 31 % [Down]
                if (trigger_level > 0)
                    trigger_level = trigger_level - 10;
                end;
            case 30 % [Up]
                if (trigger_level < 250)
                    trigger_level = trigger_level + 10;
                end;
            case '0' + 1
                trigger_channel = 1;
            case '0' + 2
                trigger_channel = 2;
            case '0' + 3
                trigger_channel = 3;
            case '0' + 4
                trigger_channel = 4;
            case 'r'
                trigger_type = 0;
            case 'f'
                trigger_type = 1;
            otherwise
                break
        end;
        TX_Settings;
        key = [];
    end;
end;

fclose(com);
close(fig);

save('Settings', 'resolution', 'trigger_channel', 'trigger_level', ...
    'trigger_type', 'trigger_position');

disp('Done');