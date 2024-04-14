; Autor reseni: Katerina Cepelkova xcepel03

; Projekt 2 - INP 2022
; Vernamova sifra na architekture MIPS64

; DATA SEGMENT
                .data
login:          .asciiz "xcepel03"  ; sem doplnte vas login
cipher:         .space  17  ; misto pro zapis sifrovaneho loginu

params_sys5:    .space  8   ; misto pro ulozeni adresy pocatku
                            ; retezce pro vypis pomoci syscall 5
                            ; (viz nize "funkce" print_string)
; a = 97, z = 122
; C -> 3, E -> 5
; xcepel03 r2-r21-r11-r9-r0-r4
; VYUZITI REGISTRU
;       r2  = pocitadlo pozice v loginu
;       r21 = znak z loginu na pozici r2
;       r11 = overflow/pomocny registr na jump
;       r9  = parita pozic (1 suda, 0 licha)
;       r0  = 0
;       r4  = vystup


;--------------------------------------------------------------------------------
; CODE SEGMENT
                .text
main:           daddi   r2, r0, 0 ; 0. pozice
                daddi   r9, r0, 0 ; licha pozice

load_loop:      lb      r21, login(r2)
                daddiu  r11, r21, -97 ; if mensi nez a -> - | else -> +
                bgez    r11, decide_loop ; if - -> konec a tisk ELSE(+) jump -> loop

main_print:     xor r21, r21, r21
                sb      r21, cipher(r2)  ; end 0
                daddi   r4, r0, cipher  ; vypis
                jal     print_string    ; vypis pomoci print_string
                syscall 0   ; halt

decide_loop:    beq r9, r0, add_c ; jump if r9 = r0 (liche cislo)
                b sub_e ; sude cislo

end_loop:       daddiu r2, r2, 1 ; r2++
                b load_loop ; zpatky na zacatek loopu

;--------------------------------------------------------------------------------
; + C
add_c:          daddiu  r21, r21, 3 ; r21 = r21 + c
                ; check for overflow
                daddiu  r11, r21, -123 ; overflow - z-1 (osetreni preteceni z)
                bgez    r11, overflow_c ; if overflow-1 >= 0 jump

end_c:          xor     r11, r11, r11 ; vycistit registr
                daddiu  r9, r9, 1 ; prehozeni na sudou
                sb      r21, cipher(r2)
                b end_loop

overflow_c:     daddiu  r21, r11, 97 ; r12 = overflow-1 + a
                b end_c
;--------------------------------------------------------------------------------
; - E 
sub_e:          daddiu  r21, r21, -5 ; r21 = r21 - e
                ; check for overflow
                daddiu  r11, r21, -97 ; overflow(r11) = r21 - a (osetreni preteceni a)
                bgez    r11, end_e ; if overflow >= 0 jump

overflow_e:     daddiu  r11, r11, 1 ; -overflow+1
                daddiu  r21, r11, 122 ; r11 = -overflow+1 + z

end_e:          xor     r11, r11, r11 ; vycistit registr
                xor r9, r9, r9 ; prehozeni na lichou 
                sb      r21, cipher(r2)
                b end_loop

;--------------------------------------------------------------------------------
print_string:   ; adresa retezce se ocekava v r4
                sw      r4, params_sys5(r0)
                daddi   r14, r0, params_sys5    ; adr pro syscall 5 musi do r14
                syscall 5   ; systemova procedura - vypis retezce na terminal
                jr      r31 ; return - r31 je urcen na return address

