#!/bin/bash

# apply kdelibs coding style to all c, cpp and header files in and below the current directory 
# 
# the coding style is defined in http://techbase.kde.org/Policies/Kdelibs_Coding_Style 
# 
# requirements: installed astyle 

astyle --indent=spaces=4 --brackets=linux \
      --indent-labels --pad=oper --unpad=paren \
      --one-line=keep-statements --convert-tabs \
      --indent-preprocessor \
      `find -type f -name '*.c'` `find -type f -name '*.cpp'` `find -type f -name '*.h'`
 
