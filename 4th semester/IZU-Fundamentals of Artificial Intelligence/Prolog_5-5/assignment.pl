% Zadani c. 29:
% Napiste program resici ukol dany predikatem u29(LIN,VOUT), kde LIN je  
% vstupni ciselny seznam obsahujici cisla z intervalu <2,20> a VOUT je  
% promenna, ve ktere se vraci pocet prvocisel v seznamu LIN. 

% Testovaci predikaty:                                 	% LOUT 
u29_1:- u29([5,3,2,4,9,12,17],VOUT),write(VOUT).	% 4
u29_2:- u29([3,5,7,11,13,17],VOUT),write(VOUT).		% 6
u29_3:- u29([6,4,12],VOUT),write(VOUT).			% 0
u29_r:- write('Zadej LIN: '),read(LIN),
	u29(LIN,VOUT),write(VOUT).

% Reseni:
u29(LIN,VOUT):-
