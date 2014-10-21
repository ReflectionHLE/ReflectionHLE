bcc makeobj.c
makeobj c audiodct.abs ..\absadict.obj _audiodict
makeobj f audiohhd.abs ..\absahead.obj _AudioHeader _audiohead
makeobj c egadict.abs ..\absedict.obj _EGAdict
makeobj f egahead.abs ..\absehead.obj EGA_grafixheader _EGAhead
makeobj f mtemp.tmp ..\absmhead.obj MapHeader _maphead

