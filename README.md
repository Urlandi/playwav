### Play PCM .wav file on PC speaker

#### Make in Linux

\#<i>(g)cc playwav64.c -o playwav64</i>

Both in 32 and 64bit arch.

#### Run

First, set eXecute flag

\#<i>chmod +x playwav64</i>

Run from prvilege mode as

\#playwav64 \<file\> [\<time multiplier\> [\<muter\>]]
- \<file\> - .wav file with PCM codec
- \<time multiplier\> - base frequency of sound, default 65000. Lower number - higher frequency.
- \<muter\> - if any char are present, there are no sound - only graph

[There is article about this on Habrahabr](https://habrahabr.ru/post/138144/)
