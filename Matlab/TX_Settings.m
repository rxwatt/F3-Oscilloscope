
% octave-compatible
% @rxwatt
command_string = zeros(8,1);
command_string(1) = '$';
command_string(2) = 'O';
command_string(3) = '<';
command_string(4) = resolution;
command_string(5) = trigger_channel;
command_string(6) = trigger_level;
command_string(7) = trigger_type;
command_string(8) = trigger_position;
command_string = uint8(command_string);
%fwrite(com,command_string);
%*** Line ABOVE works for MATLAB. Linr BELOW works for OCTAVE ***%
srl_write(com,command_string);
