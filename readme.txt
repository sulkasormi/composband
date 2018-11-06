  This file was last updated for PosChengband. Most of the advice here
  applies to FrogComposband, too; substitute FrogComposband for
  PosChengband where appropriate.

----- How to install -----

--- LINUX
  Download and unpack the source archive, or clone the git repository:
    $ git clone https://github.com/poschengband/poschengband.git

  Make sure you have the appropriate development packages installed.
  For example, you might run (Ubuntu or Mint):
    $ sudo apt-get install autoconf gcc libc6-dev libncursesw5-dev libx11-dev

  From the root of the source archive:
    $ sh autogen.sh
    $ ./configure
    $ make clean
    $ make

  To install, you may need to elevate your credentials:
    $ su
    $ make install
    $ exit

  Then run poschengband as desired:
    $ poschengband -- -n<number of windows>  ## for normal ASCII graphics
  or
    $ poschengband -g -- -n<# of windows>    ## for 8x8 tile graphics 

  You can change game windows' font, location, and size, by environment 
  variables.

  Ex.
    $ set env ANGBAND_X11_FONT '-*-*-medium-r-normal--24-*-*-*-*-*-iso8859-1'
    $ poschengband -- -n

  Then font size will be changed.

  You can set ANGBAND_X11_FONT_n for specific window which have window number n.

  Location of windows are ANGBAND_X11_AT_X_n, and ANGBAND_X11_AT_Y_n.
  Size of windows are ANGBAND_X11_COLS_n, and ANGBAND_X11_ROWS_n.

  (Thanks to Nick McConnell for implementing and improving building under Linux/MAC!)

  (ALT: For development, doing an install is undesirable. Try the following instead:
    $ sudo apt-get install clang-3.5 llvm-3.5
    $ sh autogen.sh
    $ ./configure SANITIZE_FLAGS=-fsanitize=address --with-no-install CC=clang-3.5
    $ make clean
    $ make -j4
    $ cp src/poschengband .
    $ ASAN_SYMBOLIZER_PATH=/usr/bin/llvm-symbolizer-3.5 ./poschengband -g -u<Savefile> -- -n1
    
    Note: Never pass sanitizer flags to CFLAGS as sanitizing configure causes it to fail.
    Instead, configure will append SANITIZE_FLAGS to CFLAGS and LDFLAGS once it has finished
    generating test programs. (see configure.ac)

    Note, you need to remember to copy the executable up after every build (TODO: Fix Makefile to do this!)
    Also, I like to develop with the address sanitizer enabled at all times. This works best with clang as
    your compiler, rather than gcc. Of course, 3.5 should be replaced with latest version of clang.).

    ***Fonts on Linux: My experience with linux has been that the fonts are just plain awful. Here, I document
    how I managed to install better fonts and use them in PosChengband. I spent nearly a day on this, so I'm
    hoping this might prove useful to somebody else. But mostly, it is here to remind me how I did so I don't
    stumble so much next time.

    [1] Install some better fonts. For example
    $ sudo apt-get install fonts-liberation

    [2] See what fonts are on your system that PosChengband can use:
    $ xlsfonts
    Notice that the new fonts aren't there!

    [3] Find where the new fonts were installed. For example
    $ fc-list | grep liberation
    /usr/share/fonts/truetype/liberation/LiberationSansNarrow-Italic.ttf: Liberation Sans Narrow:style=Italic
    /usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf: Liberation Sans:style=Regular
    /usr/share/fonts/truetype/liberation/LiberationMono-BoldItalic.ttf: Liberation Mono:style=Bold Italic
    /usr/share/fonts/truetype/liberation/LiberationSerif-Italic.ttf: Liberation Serif:style=Italic
    /usr/share/fonts/truetype/liberation/LiberationMono-Bold.ttf: Liberation Mono:style=Bold
    /usr/share/fonts/truetype/liberation/LiberationSansNarrow-Regular.ttf: Liberation Sans Narrow:style=Regular
    /usr/share/fonts/truetype/liberation/LiberationSerif-Bold.ttf: Liberation Serif:style=Bold
    /usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf: Liberation Mono:style=Regular
    /usr/share/fonts/truetype/liberation/LiberationSans-Italic.ttf: Liberation Sans:style=Italic
    /usr/share/fonts/truetype/liberation/LiberationSerif-BoldItalic.ttf: Liberation Serif:style=Bold Italic
    /usr/share/fonts/truetype/liberation/LiberationSansNarrow-BoldItalic.ttf: Liberation Sans Narrow:style=Bold Italic
    /usr/share/fonts/truetype/liberation/LiberationMono-Italic.ttf: Liberation Mono:style=Italic
    /usr/share/fonts/truetype/liberation/LiberationSans-BoldItalic.ttf: Liberation Sans:style=Bold Italic
    /usr/share/fonts/truetype/liberation/LiberationSerif-Regular.ttf: Liberation Serif:style=Regular
    /usr/share/fonts/truetype/liberation/LiberationSansNarrow-Bold.ttf: Liberation Sans Narrow:style=Bold
    /usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf: Liberation Sans:style=Bold

    [4] Bang head on wall in frustration!!

    [5] Tell X Server about the fonts:
    $ cd /usr/share/fonts/truetype/liberation/
    $ su
    $ mkfontscale
    $ mkfontdir
    $ exit
    $ cat fonts.dir
    ...
    LiberationMono-Regular.ttf -misc-liberation mono-medium-r-normal--0-0-0-0-m-0-adobe-standard
    LiberationMono-Regular.ttf -misc-liberation mono-medium-r-normal--0-0-0-0-m-0-ascii-0
    LiberationMono-Regular.ttf -misc-liberation mono-medium-r-normal--0-0-0-0-m-0-ibm-cp437
    ... [and many more]

    $ xset +fp /usr/share/fonts/truetype/liberation
    $ xset fp rehash
    $ xlsfonts | grep liberation
    ...
    -misc-liberation sans-medium-i-normal--0-0-0-0-p-0-iso8859-1
    -misc-liberation sans-medium-i-normal--0-0-0-0-p-0-iso8859-10
    -misc-liberation sans-medium-i-normal--0-0-0-0-p-0-iso8859-13
    -misc-liberation sans-medium-i-normal--0-0-0-0-p-0-iso8859-15
    -misc-liberation sans-medium-i-normal--0-0-0-0-p-0-iso8859-16
    ... [and many more]

    [6] Now, try to find a font you like. For example:
    $ cd [path to poschengband]
    ~/Src/poschengband> ANGBAND_X11_FONT='-misc-liberation mono-medium-r-normal--20-0-0-0-m-0-iso8859-1' ./poschengband -mx11
    You can play with the point size since these are vector fonts. I chose 20pt since my eyes suck!

    [7] Smile. Grab beer to recover from step [4].

--- Curses

  Curses is for Linux, of course, so everything said above also applies here.
  To run with a single 'big' terminal, simply run, for example:
    $ ./poschengband -mgcu -uCrusher

  To add additional terminal windows, you need to specify sub-options. You can
  configure, from the command line, a strip of terminals on the right hand side
  of the screen, or on the bottom of the screen, or both.

  For example:
    $ ./poschengband -mgcu -uCrusher -- -right 57x26,*

  This specifies that the right hand strip will be be 57 columns wide, and will
  contain 2 additional terminals. The first one, on top, will be 57x26 (i.e., 26
  rows high) while the second terminal will be 57x(LINES-26) (i.e., * means fill
  to whatever is leftover). These terminals will be numbered 1 and 2, respectively.

  Another example:
    $ ./poschengband -mgcu -uCrusher -- -bottom -bottom *x10

  This adds a bottom strip 10 rows high, and this strip will contain a single
  additional terminal window (numbered as Term-1) that will be as wide as the
  screen allows (i.e, Term-1 will be COLSx10).

  Finally, you can combine the -right and -bottom commands, in either order.
  For example:
    $ ./poschengband -mgcu -uCrusher -- -right 57x26,* -bottom *x10

  Here, Term-1 and Term-2 are on the right strip, sized as 57x26 and 57x(LINES-26),
  while Term-3 is on the bottom strip, sized (COLS-57)x10. Term-0, the Main Terminal,
  is, of course, located in the top left and sized as (COLS-57)x(LINES-10). The
  map terminal will always use as much space as possible.

  Finally, the order of the -right and -bottom command is significant, as it affects
  both the child terminal numbering and also resolves the conflict over the overlap
  region in the bottom-right corner of the screen.

  For example, the meaning of the following should now be clear:
    $ ./poschengband -mgcu -uCrusher -- -bottom *x10 -right 57x26,*

  You cannot specify more than 7 child terminals.

--- Windows

  Download the binary archive for Windows.  Unzip it to any location that you 
  will have full permissions and launch poschengband to play.

  To compile the source code in MinGW:
  $ ./autogen.sh
  $ ./configure --enable-win
  $ make

  MinGW sometimes randomly chokes on one of these steps, in which case you
  will need to redo that step.


-----  Basic for Playing  ------

  I've done quite a bit of work on the in-game documentation, though it never
  seems to be a completable task. If you are new to the game, I recommend you
  read the Newbie Guide (Press ? to activate the help system, then choose a for
  General Information followed by e). This will give a quick tutorial as well
  as some additional information about the game. Helpfiles are also available
  as html files for viewing outside the game in your favorite browser. In this
  case, open lib/help/html/tang.html.

---------  Commands  ----------- 

  Again, please refer to the in-game help (It hurts me to keep the same information
  up to date in multiple places). Press ? for help and then k for Commands. Choose
  the topic that interests you (Command Descriptions for an overview, or one of the
  Keyset options for reference on which keys to press).

  Or open lib/help/html/commdesc.html or lib/help/html/command.html in your browser.

