%ifdef __WIN32__

%imacro EXTSYM 1-*
%rep %0
  	extern _%1
  	%define %1 _%1
%rotate 1
%endrep
%endmacro

%imacro NEWSYM 1
  global _%1
  _%1:
  %1:
%endmacro

%else

%define EXTSYM extern

%imacro NEWSYM 1
	global %1
	%1:
%endmacro

%endif
