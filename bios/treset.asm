;THE COMPUTER CODE CONTAINED HEREIN IS THE SOLE PROPERTY OF PARALLAX
;SOFTWARE CORPORATION ("PARALLAX").  PARALLAX, IN DISTRIBUTING THE CODE TO
;END-USERS, AND SUBJECT TO ALL OF THE TERMS AND CONDITIONS HEREIN, GRANTS A
;ROYALTY-FREE, PERPETUAL LICENSE TO SUCH END-USERS FOR USE BY SUCH END-USERS
;IN USING, DISPLAYING,  AND CREATING DERIVATIVE WORKS THEREOF, SO LONG AS
;SUCH USE, DISPLAY OR CREATION IS FOR NON-COMMERCIAL, ROYALTY OR REVENUE
;FREE PURPOSES.  IN NO EVENT SHALL THE END-USER USE THE COMPUTER CODE
;CONTAINED HEREIN FOR REVENUE-BEARING PURPOSES.  THE END-USER UNDERSTANDS
;AND AGREES TO THE TERMS HEREIN AND ACCEPTS THE SAME BY USE OF THIS FILE.  
;COPYRIGHT 1993-1998 PARALLAX SOFTWARE CORPORATION.  ALL RIGHTS RESERVED.
; This resets the timer rat to 18.2 Hz.
; to compile, just type: 
; ml treset.asm
; the compiler generates treset.com

.MODEL TINY
.CODE
.STARTUP

TDATA       EQU 40h
TCOMMAND    EQU 43h

mov     al, 36h
out     TCOMMAND, al    ; Reser PIT channel 0
mov     ax, 0h
out     TDATA, al
mov     al, ah
out     TDATA, al

.EXIT
END


