function TX_Setting( handles )
%TX_S Summary of this function goes here
%   Detailed explanation goes here
command_string = zeros(8,1);
command_string(1) = '$';
command_string(2) = 'O';
command_string(3) = '<';
command_string(4) = handles.resolution;
command_string(5) = handles.trigger_channel;
command_string(6) = handles.trigger_level;
command_string(7) = handles.trigger_type;
command_string(8) = handles.trigger_position;
command_string = uint8(command_string);
fwrite(handles.com,command_string);

end

