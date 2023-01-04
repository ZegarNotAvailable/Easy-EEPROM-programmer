;PROGRAM KOPIOWANIA DANYCH DO EEPROM (NP. KM28C64A) DLA CA80
        .cr z80                     
        .tf PR_EEPROM_SAMSUNG.hex,int   
        .lf PR_EEPROM_SAMSUNG.lst
        .sf PR_EEPROM_SAMSUNG.sym       
        .in ca80.inc                
        .sm code                ; 
        .or $c000               ; U12/C000
.e_SDP        
        jp enable_SDP           ; ustaw Software Data Protection
.d_SDP        
        jp disable_SDP          ; odblokuj SDP
        jp copy                ; kopiuj
        
.compare:                       ;porównaj
        ld  SP,$ff66            ; 
        ld BC,2000H             ; BC-licznik (8 kB)
        ld HL,0000H             ; adres oryginalu (U11)
        ld DE,4000H             ; adres kopii (u10)
.next:
        ld A,(DE)
        inc DE
        cpi
        jr Z,.equal
        push BC
        dec HL
        call LADR
        .db 44h
        inc HL
        pop BC
.equal:
        xor A
        xor B
        jr NZ,.next
        xor C
        jr NZ,.next
        push BC
        pop HL
        call LADR
        .db 44h
.end:		
        halt                        ; koniec zabawy. :-)
        jr .end
		
copy:        
        ld  SP,$ff66                ; 
        ld BC,2000H                 ; BC-licznik (8 kB)
        ld HL,8000H                 ; adres oryginalu (U11)
        ld DE,4000H                 ; adres kopii (u10)
        halt                        ; <2ms (dla pewnosci, bo mozemy trafic tuz przed NMI)
.loop:
        ld A,40h                    ; zapis 64 bajtów jednoczesnie
.loop1:
        ldi       	                ; kopiuj bajt    
        dec A                       ; Tlbc (load byte cycle 200 ns max 150 us) - musimy sie spieszyc
        jr nz,.loop1                ; czy juz 64?
        xor C                       ; sprawdz czy koniec
        jr nz,.next                 ; jeszcze nie
        xor B
        jr z,end                    ; teraz tak
.next:
        push BC                     ; 
        call ladr                   ; pokaz HL
        .db $44                     ; 
        pop BC 
        halt                        ; 2ms (nota katalogowa: czas zapisu MAX 5 ms)
        halt                        ; 2ms
        halt                        ; 2ms 
        jr .loop                    ;                        
end:
        call ladr                   ; pokaz HL
        .db $44
.end:		
        halt                        ; koniec zabawy. :-)
        jr .end
        ;
enable_SDP:
        ld  SP,$ff66                ;
        ld A,55H                    ;
        ld HL,5555H                 ; adres 
        ld (HL),0AAH                ; klucz
        ld DE,4AAAH                 ; adres 
        ld (DE),A                   ; klucz
        ld (HL),0A0H                ; klucz
        ld HL,5FFFH                 ; dowolny adres (ostatnia komórka)
        ld (HL),0FFH                ; w monitorze CA80 jest FF
        call ladr                   ; pokaż HL
        .db $44              
.end:
        halt                        ; Pamięć już jest zabezpieczona
        jr .end
		;
disable_SDP:
        ld  SP,$ff66                ; 
        ld A,55H
        ld HL,5555H                 ; adres 
        ld (HL),0AAH                ; klucz
        ld DE,4AAAH                 ; adres 
        ld (DE),A                   ; klucz
        ld (HL),080H                ; klucz
        ld (HL),0AAH                ; klucz
        ld (DE),A                   ; klucz 
        ld (HL),020H                ; klucz
        ld HL,5FFFH                 ; dowolny adres (ostatnia komórka)
        ld (HL),0FFH                ; w monitorze CA80 jest FF
        call ladr                   ; pokaż HL
        .db $44              
.end:
        halt                        ; Pamięć już jest odbezpieczona
        jr .end