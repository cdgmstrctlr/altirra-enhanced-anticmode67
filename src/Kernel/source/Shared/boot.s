;	Altirra - Atari 800/800XL/5200 emulator
;	Modular Kernel ROM - Boot Code
;	Copyright (C) 2008-2016 Avery Lee
;
;	Copying and distribution of this file, with or without modification,
;	are permitted in any medium without royalty provided the copyright
;	notice and this notice are preserved.  This file is offered as-is,
;	without any warranty.

;==========================================================================
; Disk boot routine.
;
; Exit:
;	DBUFLO/DBUFHI = $0400	(Undoc; required by Ankh and the 1.atr SMB demo)
;	Last sector in $0400	(Undoc; required by Ankh)
;
.proc BootDisk
	;issue a status request first to see if the disk is active; if this
	;doesn't come back, don't bother trying to read
	lda		#$53
	sta		dcomnd
	ldx		#1
	stx		dunit
	jsr		dskinv
	bpl		status_ok
xit:
	rts

status_ok:
.if _KERNEL_USE_BOOT_SCREEN
	jsr		try_read_sector1
	bpl		try_boot_loop_postsector1
enter_exit_boot_screen:
	lda		portb
	eor		#$80
	sta		portb
	bmi		try_boot_loop
	jsr		BootScreen.boot_entry
	jmp		enter_exit_boot_screen

try_boot_loop:
	jsr		try_read_sector1
	smi
try_boot_loop_postsector1:
	jsr		try_boot
	jsr		BootShowError
	jmp		try_boot_loop

.else
try_boot_loop:
.endif

try_read_sector1:
	;read first sector to $0400
	ldx		#1
	stx		dunit
	stx		daux1
	dex
	stx		dbuflo
	stx		daux2
	mva		#$52	dcomnd
	mva		#$04	dbufhi

.if _KERNEL_USE_BOOT_SCREEN
	jmp		dskinv
.else
	jsr		dskinv
	bmi		fail
.endif

try_boot:
	ldx		#dosini
	jsr		BootInitHeaders

	;load remaining sectors
sectorloop:
	;copy sector from $0400 to destination (required by Ankh; see above)
	;bump destination address for next sector copy
	jsr		BootCopyBlock

	;exit if this was the last sector (note that 0 means to load 256 sectors!)
	dec		dbsect
	beq		loaddone
	
	;increment sector (yes, this can overflow to 256)
	inw		daux1

	;read next sector
	jsr		dskinv
	
	;keep going if we succeeded
	bpl		sectorloop
	
	;read failed
fail:
.if _KERNEL_USE_BOOT_SCREEN
	jmp		BootShowError
.else
	jsr		BootShowError
	jmp		try_boot_loop
.endif

loaddone:
	jsr		BootRunLoader
	bcs		fail
	
	;Diskette Boot Process, step 7 (p.161 of the OS Manual) is misleading. It
	;says that DOSVEC is invoked after DOSINI, but actually that should NOT
	;happen here -- it happens AFTER cartridges have had a chance to run.
	;This is necessary for BASIC to gain control before DOS goes to load
	;DUP.SYS.
	jsr		InitDiskBoot

	;Must not occur until after init routine is called -- SpartaDOS 3.2 does
	;an INC on this flag and never exits.
	mva		#1 boot?
.if _KERNEL_USE_BOOT_SCREEN
	pla
	pla
.endif
	rts
.endp


;============================================================================

.proc BootCassette
	;set continuous mode -- must do this as CSOPIV doesn't
	lda		#$80
	sta		ftype

	;open cassette device
	jsr		csopiv
	
	;read first block
	jsr		rblokv
	bmi		load_failure
	
	ldx		#casini
	jsr		BootInitHeaders
	
block_loop:
	;copy 128 bytes from CASBUF+3 ($0400) to destination
	;update destination pointer
	jsr		BootCopyBlock

	;Read next block.
	;
	;We always need to do one more to catch the EOF block, which is
	;required by STDBLOAD2. It does not matter whether this last read
	;succeeds -- timeout, checksum error, not EOF, doesn't matter to
	;the stock OS.
	jsr		rblokv
	
	dec		dbsect
	beq		load_done

	tya
	bpl		block_loop

load_failure:
	lda		#0
	sta		ckey
	jsr		CassetteClose
	jmp		BootShowError

load_done:
	;run loader
	jsr		BootRunLoader
	bcs		load_failure

	;run cassette init routine
	jsr		InitCassetteBoot

	;clear cassette boot key flag
	lda		#0
	sta		ckey

	;set cassette boot flag
	mva		#2 boot?
	
	;run application
	jmp		(dosvec)
.endp

;============================================================================
.proc BootInitHeaders
	;copy the first four bytes to DFLAGS, DBSECT, and BOOTAD
	ldy		#$fc
	mva:rne	$0400-$fc,y dflags-$fc,y+
	
	;copy boot address in BOOTAD to BUFADR
	sta		bufadr+1
	lda		bootad
	sta		bufadr
	
	;copy init vector
	mwa		$0404 0,x
	rts
.endp

;============================================================================
.proc BootRunLoader
	;Some tape versions of River Raid is sensitive to both the A
	;register value on boot and RAMLO, for some reason. For compatibility,
	;we need to return the high byte in A and RAMLO needs to contain the
	;run address.

	lda		bootad
	add		#$06			;loader is at load address + 6
	sta		ramlo
	lda		bootad+1
	adc		#0
	sta		ramlo+1
	jmp		(ramlo)
.endp

;============================================================================
.proc BootCopyBlock
	ldy		#$7f
	mva:rpl	$0400,y (bufadr),y-

	lda		bufadr
	eor		#$80
	sta		bufadr
	smi:inc	bufadr+1
	rts
.endp

;============================================================================

.proc BootShowError
	ldx		#$f5
msgloop:
	txa
	pha
	lda		errormsg-$f5,x
	jsr		EditorPutByte
	pla
	tax
	inx
	bne		msgloop
	rts
	
errormsg:
	dta		'BOOT ERROR',$9B
.endp
