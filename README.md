# Alpha Tool #

**Alpha Tool &ndash; computes alpha-channel from image pixel differences**  
**Copyright (C) 2015 LoRd_MuldeR <<MuldeR2@GMX.de>>. Some Rights Reserved.**

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License <http://www.gnu.org/>.
	Note that this program is distributed with ABSOLUTELY NO WARRANTY.

&nbsp;

## Example: ##

As a picture is worth a thousand words (changes are in area around the photograph!):

*Input File #1:*  
![source_1](img/source_1.png)

*Input File #2:*  
![source_2](img/source_2.png)

*Difference Map:*  
![diff_map](img/diff_map.png)

*Final Result:*  
![result](img/result.png)

&nbsp;  

## Usage: ##

Alpha Tool command-line syntax:

	Usage:
	   AlphaTool.exe <in_1.png> <in_2.png> <out.png> [<mix_mode>] [<map.png>]
	   
	Modes:
	   average, luminosity, lightness

&nbsp;  
Example command-line:

	AlphaTool.exe "C:\Foo\Input_1.png" "C:\Foo\Input_2.png" "C:\Bar\Output.png"

&nbsp;  
For information about the different "mixing" modes, please have a look [**here**](https://docs.gimp.org/2.6/en/gimp-tool-desaturate.html).

&nbsp;  
&nbsp;  

e.o.f.
