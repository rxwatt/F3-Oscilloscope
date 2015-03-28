function varargout = MainWindow(varargin)
% MAINWINDOW MATLAB code for MainWindow.fig
%      MAINWINDOW, by itself, creates a new MAINWINDOW or raises the existing
%      singleton*.
%
%      H = MAINWINDOW returns the handle to a new MAINWINDOW or the handle to
%      the existing singleton*.
%
%      MAINWINDOW('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in MAINWINDOW.M with the given input arguments.
%
%      MAINWINDOW('Property','Value',...) creates a new MAINWINDOW or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before MainWindow_OpeningFcn gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to MainWindow_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help MainWindow

% Last Modified by GUIDE v2.5 28-Mar-2015 11:47:58

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
    'gui_Singleton',  gui_Singleton, ...
    'gui_OpeningFcn', @MainWindow_OpeningFcn, ...
    'gui_OutputFcn',  @MainWindow_OutputFcn, ...
    'gui_LayoutFcn',  [] , ...
    'gui_Callback',   []);
if nargin && ischar(varargin{1})
    gui_State.gui_Callback = str2func(varargin{1});
end

if nargout
    [varargout{1:nargout}] = gui_mainfcn(gui_State, varargin{:});
else
    gui_mainfcn(gui_State, varargin{:});
end
% End initialization code - DO NOT EDIT



% --- Executes just before MainWindow is made visible.
function MainWindow_OpeningFcn(hObject, eventdata, handles, varargin)

% Choose default command line output for MainWindow
handles.output = hObject;

% Oscilloscope settings
handles.adc_sample_length = 8000/2;
handles.resolution = 13;
handles.trigger_channel = 2;
handles.trigger_level = 120;
handles.trigger_type = 0;
handles.trigger_position = 5;
handles.strings = ['500 ns'; '  1 us'; '  2 us'; '  5 us'; ' 10 us'; ' 20 us';...
    ' 50 us'; '100 us'; '200 us'; '500 us'; '  1 ms'; '  2 ms';...
    '  5 ms'; ' 10 ms'; ' 20 ms'; ' 50 ms'; '100 ms'];

com_port = '/dev/tty.usbmodemfd131';
%com_port = 'com8';
handles.com = serial(com_port,'BaudRate',115200,'InputBufferSize',32768,'Timeout',5);
fopen(handles.com);
TX_Settings(handles);

% Update handles structure
guidata(hObject, handles);


% --- Outputs from this function are returned to the command line.
function varargout = MainWindow_OutputFcn(hObject, eventdata, handles)
% Get default command line output from handles structure
varargout{1} = handles.output;


% --- Executes on selection change in menuChannel.
function menuChannel_Callback(hObject, eventdata, handles)


% --- Executes during object creation, after setting all properties.
function menuChannel_CreateFcn(hObject, eventdata, handles)


% --- Executes on button press in chkEnable.
function chkEnable_Callback(hObject, eventdata, handles)


% --- Executes on selection change in menuTriggerChannel.
function menuTriggerChannel_Callback(hObject, eventdata, handles)
disp(get(menuTrigger,'Value'));
%handles.trigger_channel =  
%TX_Settings(handles);
%guidata(hObject, handles);

% --- Executes on button press in btnHorizMinus.
function btnHorizMinus_Callback(hObject, eventdata, handles)
if (handles.resolution < (size(handles.strings,1)-1))
    handles.resolution = handles.resolution + 1;
    TX_Settings(handles);
end;
guidata(hObject, handles);

% --- Executes on button press in btnHorizPlus.
function btnHorizPlus_Callback(hObject, eventdata, handles)
if (handles.resolution > 0)
    handles.resolution = handles.resolution - 1;
    TX_Settings(handles);
end;
guidata(hObject, handles);

% --- Executes on button press in btnHorizRight.
function btnHorizRight_Callback(hObject, eventdata, handles)
if (handles.trigger_position < 10)
    handles.trigger_position = handles.trigger_position + 1;
    guidata(hObject, handles);
    TX_Settings(handles);
end;


% --- Executes on button press in btnHorizLeft.
function btnHorizLeft_Callback(hObject, eventdata, handles)
if (handles.trigger_position > 0)
    handles.trigger_position = handles.trigger_position - 1;
    guidata(hObject, handles);
    TX_Settings(handles);
end;


%--- Executes on button press in btnVertUp.
function btnVertUp_Callback(hObject, eventdata, handles)


% --- Executes on button press in btnVertDown.
function btnVertDown_Callback(hObject, eventdata, handles)


% --- Executes on button press in btnVertMinus.
function btnVertMinus_Callback(hObject, eventdata, handles)


% --- Executes on button press in btnVertPlus.
function btnVertPlus_Callback(hObject, eventdata, handles)


% --- Executes on selection change in menuTriggerType.
function menuTriggerType_Callback(hObject, eventdata, handles)


% --- Executes on button press in btnTriggerUp.
function btnTriggerUp_Callback(hObject, eventdata, handles)
if (handles.trigger_level < 250)
    handles.trigger_level = handles.trigger_level + 10;
    guidata(hObject, handles);
    TX_Settings(handles);
end;


% --- Executes on button press in btnTriggerDown.
function btnTriggerDown_Callback(hObject, eventdata, handles)
if (handles.trigger_level > 0)
    handles.trigger_level = handles.trigger_level - 10;
    guidata(hObject, handles);
    TX_Settings(handles);
end;


% --- Executes on button press in btnStart.
function btnStart_Callback(hObject, eventdata, handles)
handles.running = 1;
guidata(hObject,handles);
while (1)
    handles = guidata(hObject);
    % Receive data
    try
        fwrite(handles.com,'$O>');
    catch err
        break;
    end;
    ADC_Header = fread(handles.com,4,'uint8');
    handles.adc_mode = ADC_Header(1);
    handles.adc_sample_length = ADC_Header(2)*256 + ADC_Header(3);
    handles.adc_triggered = ADC_Header(4);
    ADC_Buffer = fread(handles.com,handles.adc_sample_length*4,'uint8');    
    % Sort data
    ADC_Data = zeros(handles.adc_sample_length,4);
    indexes = 1:2:handles.adc_sample_length*2;
    ADC_Data(:,1) = ADC_Buffer(indexes);
    ADC_Data(:,2) = ADC_Buffer(indexes+1);
    ADC_Data(:,3) = ADC_Buffer(indexes+handles.adc_sample_length*2);
    ADC_Data(:,4) = ADC_Buffer(indexes+1+handles.adc_sample_length*2);
    DisplayWave(ADC_Data, handles);
end;


% --- Executes on button press in btnStop.
function btnStop_Callback(hObject, eventdata, handles)
handles.running = 0;
guidata(hObject,handles);


% --- Executes when user attempts to close figureOscil.
function figureOscil_CloseRequestFcn(hObject, eventdata, handles)
% hObject    handle to figureOscil (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
handles.running = 0;
fclose(handles.com);
guidata(hObject,handles);
disp('Closing');
pause(1);
delete(hObject); % Hint: delete(hObject) closes the figure


% --- Executes on key press with focus on figureOscil and none of its controls.
function figureOscil_KeyPressFcn(hObject, eventdata, handles)
key = uint8(get(hObject,'currentchar'));
if (key == 27)
    figureOscil_CloseRequestFcn(hObject, eventdata, handles);
end;
