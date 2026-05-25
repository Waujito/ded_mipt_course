set name=%1
tasm /la %name%.asm
tlink /t %name%.obj
%name%.com
