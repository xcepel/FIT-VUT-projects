-- VUT FIT - IDS 2022/2023 - 2. cast projektu
-- Autori:
--      Katerina Cepelkova xcepel03
--      Tomas Ebert xebert00

--- DROP tabulek + sekvenci ---
DROP TABLE Hvezda CASCADE CONSTRAINTS;
DROP TABLE Planetarni_system CASCADE CONSTRAINTS;
DROP TABLE Planeta CASCADE CONSTRAINTS;
DROP TABLE Slozeni_atmosfery CASCADE CONSTRAINTS;
DROP TABLE Prvek CASCADE CONSTRAINTS;
DROP TABLE Tvori CASCADE CONSTRAINTS;
DROP TABLE Ma_na_orbite CASCADE CONSTRAINTS;
DROP TABLE Flotila CASCADE CONSTRAINTS;
DROP TABLE Lod CASCADE CONSTRAINTS;
DROP TABLE Rytir_Jedi CASCADE CONSTRAINTS;
DROP TABLE Nakladni CASCADE CONSTRAINTS;
DROP TABLE Utocna CASCADE CONSTRAINTS;
DROP TABLE Velitelska CASCADE CONSTRAINTS;

DROP SEQUENCE system_seq;
DROP SEQUENCE hvezda_seq;
DROP SEQUENCE planeta_seq;
DROP SEQUENCE flotila_seq;
DROP SEQUENCE lod_seq;
DROP SEQUENCE jedi_seq;

DROP PROCEDURE flotila_spatny_nazev;
DROP PROCEDURE pocet_midich_flotila;

DROP MATERIALIZED VIEW rytiri_a_lode;

--- Vytvareni tabulek + sekvenci + triggeru ---
CREATE TABLE Planetarni_system (
    ID_system INTEGER PRIMARY KEY,
    jmeno VARCHAR(20) NOT NULL,
    datum_objeveni DATE NOT NULL
);

CREATE SEQUENCE system_seq;

CREATE OR REPLACE TRIGGER ID_system_generator
    BEFORE INSERT ON Planetarni_system
    FOR EACH ROW
    WHEN (NEW.ID_system IS NULL)
BEGIN
    SELECT system_seq.NEXTVAL
    INTO :NEW.ID_system
    FROM DUAL;
END; 
/

CREATE TABLE Hvezda (
    ID_hvezda INTEGER PRIMARY KEY,
    typ VARCHAR(20) NOT NULL, -- ('Bily trpaslik', 'Hnedy trpaslik', 'Jasny obr', 'Supernova', 'Nova', 'Pulsar', 'Obr', 'Podobr', 'Veleobr')
    ID_system INTEGER NOT NULL,

CONSTRAINT Hvezda_System_FK FOREIGN KEY (ID_system) 
    REFERENCES Planetarni_system ON DELETE CASCADE
);

CREATE SEQUENCE hvezda_seq;

CREATE OR REPLACE TRIGGER ID_hvezda_generator
    BEFORE INSERT ON Hvezda
    FOR EACH ROW
    WHEN (NEW.ID_hvezda IS NULL)
BEGIN
    SELECT hvezda_seq.NEXTVAL
    INTO :NEW.ID_hvezda
    FROM DUAL;
END; 
/

CREATE TABLE Planeta (
    ID_planeta INTEGER PRIMARY KEY,
    jmeno VARCHAR(20) NOT NULL,
    vzdalenost_od_slunce INTEGER CHECK (vzdalenost_od_slunce > 0) NOT NULL,
    hmotnost INTEGER NOT NULL,
    pocet_obyvatel INTEGER NOT NULL,
    typ VARCHAR(20) NOT NULL, -- ('Horky Jupiter', 'Obri planeta', 'Superzeme', 'Teresticka planeta', 'Ledovy obr', 'Plynny obr')
    ID_system INTEGER NOT NULL,

CONSTRAINT Planeta_System_FK FOREIGN KEY (ID_system) 
    REFERENCES Planetarni_system ON DELETE CASCADE
);

CREATE SEQUENCE planeta_seq;

CREATE OR REPLACE TRIGGER ID_planeta_generator
    BEFORE INSERT ON Planeta
    FOR EACH ROW
    WHEN (NEW.ID_planeta IS NULL)
BEGIN
    SELECT planeta_seq.NEXTVAL
    INTO :NEW.ID_planeta
    FROM DUAL;
END; 
/

CREATE TABLE Prvek (
    nazev VARCHAR(20) PRIMARY KEY,
    chem_vzorec VARCHAR(20) NOT NULL
);

CREATE TABLE Slozeni_atmosfery (
-- todo ID
    ID_planeta INTEGER,
    nazev VARCHAR(20) NOT NULL,
    percent_zastoupeni INTEGER NOT NULL,

CONSTRAINT Slozeni_atmosfery_Planeta_FK FOREIGN KEY (ID_planeta)
    REFERENCES Planeta ON DELETE CASCADE,
CONSTRAINT Slozeni_atmosfery_Prvek_FK FOREIGN KEY (nazev)
    REFERENCES Prvek ON DELETE CASCADE,
    
CONSTRAINT Slozeni_atmosfery_PK PRIMARY KEY (ID_planeta,nazev)
);

CREATE TABLE Tvori (
-- todo ID
    ID_hvezda INTEGER,
    nazev VARCHAR(20) NOT NULL,

CONSTRAINT Tvori_Hvezda_FK FOREIGN KEY (ID_hvezda)
    REFERENCES Hvezda ON DELETE CASCADE,
CONSTRAINT Tvori_Prvek_FK FOREIGN KEY (nazev)
    REFERENCES Prvek ON DELETE CASCADE,
    
CONSTRAINT Tvori_PK PRIMARY KEY (ID_hvezda,nazev)
);

CREATE TABLE Flotila (
    ID_flotila INTEGER PRIMARY KEY, 
    nazev VARCHAR(20) NOT NULL, 
    typ VARCHAR(20) NOT NULL, --('Valecna', 'Dobyvaci', 'Zasobovaci', 'Konstrukcni', 'Kuryrni', 'Paseracka', 'Velitelska')
    datum_vytvoreni DATE NOT NULL
);

CREATE SEQUENCE flotila_seq;

CREATE OR REPLACE TRIGGER ID_flotila_generator
    BEFORE INSERT ON Flotila
    FOR EACH ROW
    WHEN (NEW.ID_flotila IS NULL)
BEGIN
    SELECT flotila_seq.NEXTVAL
    INTO :NEW.ID_flotila
    FROM DUAL;
END; 
/

CREATE TABLE Ma_na_orbite (
-- todo ID
    ID_planeta INTEGER,
    ID_flotila INTEGER,

CONSTRAINT Ma_na_orbite_Planeta_FK FOREIGN KEY (ID_planeta)
    REFERENCES Planeta ON DELETE CASCADE,
CONSTRAINT Ma_na_orbite_Flotila_FK FOREIGN KEY (ID_flotila)
    REFERENCES Flotila ON DELETE CASCADE,
    
CONSTRAINT Ma_na_orbite_PK PRIMARY KEY (ID_planeta,ID_flotila)
);

CREATE TABLE Lod (
    ID_lod INTEGER PRIMARY KEY,
    typ VARCHAR(20) NOT NULL, --('Bombarder', 'Raketoplan', 'Kravela', 'Korzar', 'Unikovy modul', 'Flotilovy prepravce', 'Hvezdny bojovnik', 'Vesmirna stanice', 'Vysadkova lod')
    datum_vytvoreni DATE NOT NULL,
    ID_flotila INTEGER NOT NULL,

CONSTRAINT Lod_Flotila_FK FOREIGN KEY (ID_flotila)
    REFERENCES Flotila ON DELETE CASCADE
);

CREATE SEQUENCE lod_seq;

CREATE OR REPLACE TRIGGER ID_lod_generator
    BEFORE INSERT ON Lod
    FOR EACH ROW
    WHEN (NEW.ID_lod IS NULL)
BEGIN
    SELECT lod_seq.NEXTVAL
    INTO :NEW.ID_lod
    FROM DUAL;
END; 
/

CREATE TABLE Rytir_Jedi (
    ID_jedi INTEGER PRIMARY KEY, -- == r_cislo
    jmeno VARCHAR(20) NOT NULL,
    mnozstvi_midi_chlorianu INTEGER NOT NULL,
    barva_svetelneho_mece VARCHAR(10) CHECK (barva_svetelneho_mece = 'Modra' OR barva_svetelneho_mece = 'Cervena' OR barva_svetelneho_mece = 'Zelena') NOT NULL,
    datum_narozeni DATE NOT NULL,
    ID_flotila INTEGER NOT NULL,
    ID_lod INTEGER,
    padawan INTEGER,

CONSTRAINT Rytir_Jedi_Flotila_FK FOREIGN KEY (ID_flotila)
    REFERENCES Flotila ON DELETE CASCADE,
CONSTRAINT Rytir_Jedi_Lod_FK FOREIGN KEY (ID_lod)
    REFERENCES Lod,
CONSTRAINT Rytir_Jedi_padawan_FK FOREIGN KEY (padawan)
    REFERENCES Rytir_Jedi ON DELETE CASCADE
);

CREATE SEQUENCE jedi_seq;

CREATE OR REPLACE TRIGGER ID_jedi_generator
    BEFORE INSERT ON Rytir_jedi
    FOR EACH ROW
    WHEN (NEW.ID_jedi IS NULL)
BEGIN
    SELECT jedi_seq.NEXTVAL
    INTO :NEW.ID_jedi
    FROM DUAL;
END; 
/

--- Generalizace/specializace ---
-- tabulka pro nadtyp + pro podtypy s primarnim klicem nadtypu
CREATE TABLE Nakladni (
    ID_lod INTEGER,
    typ_nakladu VARCHAR(20) NOT NULL,
    kapacita INTEGER NOT NULL,
    
CONSTRAINT Nakladni_PK FOREIGN KEY (ID_lod) 
    REFERENCES Lod ON DELETE CASCADE
);

CREATE TABLE Utocna (
    ID_lod INTEGER,
    munice VARCHAR(20) NOT NULL, --('Laser', 'Superlaser', 'Protonove delo', 'Parazni raketa', 'Vrtaci hrot', 'Flak kanon')
    
CONSTRAINT Utocna_PK FOREIGN KEY (ID_lod) 
    REFERENCES Lod ON DELETE CASCADE
);

CREATE TABLE Velitelska (
    ID_lod INTEGER,
    systemy VARCHAR(20), --('GPS','Radar', 'Generator', 'Elektromagenticke pole', 'Rizeni raket', 'Stity', 'Navigatory citlive na silu', 'Jump-by-jump navigace')
    dulezitost INTEGER, -- nic = nedulezita
    
CONSTRAINT Velitelska_PK FOREIGN KEY (ID_lod) 
    REFERENCES Lod ON DELETE CASCADE
);
-- Vysvetleni pouziti
--- nevyuzili jsme "pouze tabulky pro podtypy i s atributy nadtypu, nebot je moznost, ze jedna lod je muze byt napr. velitelska i utocna" 
--- u ostatnich verzi reseni by byla zbytecna starost s hodnotami NULL

COMMIT;


--- Vkladani do tabulek ---
INSERT INTO Planetarni_system
    VALUES(NULL,'Galakticke jadro',to_date('24-08-2000','dd-mm-yyyy'));
INSERT INTO Planetarni_system
    VALUES(2,'Vnejsi okraj',to_date('12-08-2042','dd-mm-yyyy'));

INSERT INTO Planeta
    VALUES(NULL,'Alderaan',150000000,420000000,1715175517,'Teresticka planeta',1);
INSERT INTO Planeta
    VALUES(NULL,'Corellia',35000000,827600145,74000000,'Teresticka planeta',1);
INSERT INTO Planeta
    VALUES(NULL,'Tatooine',107000000,800001,17539510,'Superzeme',2);
INSERT INTO Planeta
    VALUES(NULL,'Endor',57000000,999999999,75111,'Obri planeta',2);
INSERT INTO Planeta
    VALUES(NULL,'Mustafar',70000000,634521,645,'Horky Jupiter',2);

INSERT INTO Hvezda
    VALUES(NULL,'Supernova',1);
INSERT INTO Hvezda
    VALUES(NULL,'Bily trpaslik',2);
INSERT INTO Hvezda
    VALUES(NULL,'Bily trpaslik',2);

INSERT INTO Prvek
    VALUES('Vodik','H');
INSERT INTO Prvek
    VALUES('Kyslik','O');
    
INSERT INTO Slozeni_atmosfery
    VALUES (3,'Kyslik',35);
INSERT INTO Slozeni_atmosfery
    VALUES (3,'Vodik',20); 
INSERT INTO Slozeni_atmosfery
    VALUES (4,'Kyslik',78); 
INSERT INTO Slozeni_atmosfery
    VALUES (2,'Kyslik',21);
INSERT INTO Slozeni_atmosfery
    VALUES (5,'Vodik',100); 
  
INSERT INTO Tvori
    VALUES (1,'Kyslik'); 
INSERT INTO Tvori
    VALUES (1,'Vodik'); 
    
INSERT INTO Flotila
    VALUES (NULL, 'ABC_123', 'Valecna', to_date('21-03-2050','dd-mm-yyyy')); 
INSERT INTO Flotila
    VALUES (NULL, 'XSE_960', 'Obranna', to_date('21-03-2048','dd-mm-yyyy')); 
INSERT INTO Flotila
    VALUES (NULL, 'BBC_444', 'Paseracka', to_date('21-03-2065','dd-mm-yyyy')); 
INSERT INTO Flotila
    VALUES (NULL, 'Flotilka', 'Paseracka', to_date('21-05-2066','dd-mm-yyyy')); 
    
Insert INTO Ma_na_orbite
    VALUES (4,1);
Insert INTO Ma_na_orbite
    VALUES (4,2);
Insert INTO Ma_na_orbite
    VALUES (3,3);
 
INSERT INTO Lod
    VALUES (NULL,'X-Wing',to_date('21-03-2076','dd-mm-yyyy'),1);   
INSERT INTO Lod
    VALUES (NULL,'Milenum Falcon',to_date('12-09-2086','dd-mm-yyyy'),1);
INSERT INTO Lod
    VALUES (NULL,'Hvezda Smrti',to_date('12-09-2040','dd-mm-yyyy'),2);
INSERT INTO Lod
    VALUES (NULL,'X-Wing',to_date('21-03-2066','dd-mm-yyyy'),3);  

INSERT INTO Utocna
    VALUES (1,'Laser');
INSERT INTO Utocna
    VALUES (3,'UltraLaser');    
INSERT INTO Utocna
    VALUES (4,'Laser');    
    
INSERT INTO Nakladni
    VALUES (2,'Zbrane',500);

Insert INTO Rytir_jedi
    VALUES (NULL,'Darth Vader',85,'Cervena',to_date('15-03-2010','dd-mm-yyyy'),1,1,NULL);
Insert INTO Rytir_jedi
    VALUES (NULL,'Obi Van Kenobi',70,'Zelena',to_date('01-05-2000','dd-mm-yyyy'),2,4,NULL);
Insert INTO Rytir_jedi
    VALUES (NULL,'Luke Skywalker',87,'Modra',to_date('11-12-2030','dd-mm-yyyy'),2,2,2);
Insert INTO Rytir_jedi
    VALUES (NULL,'Rey',59,'Modra',to_date('11-12-2060','dd-mm-yyyy'),1,NULL,3);
Insert INTO Rytir_jedi
    VALUES (NULL,'Kylo Ren',69,'Cervena',to_date('11-12-2058','dd-mm-yyyy'),3,NULL,NULL);
    
--- Tisk tabulek ---
--SELECT * FROM Planetarni_system;
--SELECT * FROM Planeta;
--SELECT * FROM Hvezda;
--SELECT * FROM Prvek;
--SELECT * FROM Slozeni_atmosfery;
--SELECT * FROM Tvori;
--SELECT * FROM Flotila;
--SELECT * FROM Ma_na_orbite;
--SELECT * FROM Rytir_jedi;
--SELECT * FROM Lod;
--SELECT * FROM Nakladni;
--SELECT * FROM Utocna;
--SELECT * FROM Velitelska;

-- Ukoncovani triggeru pro generaci ID
ALTER TRIGGER ID_jedi_generator DISABLE;
ALTER TRIGGER ID_lod_generator DISABLE;
ALTER TRIGGER ID_flotila_generator DISABLE;
ALTER TRIGGER ID_planeta_generator DISABLE;
ALTER TRIGGER ID_hvezda_generator DISABLE;
ALTER TRIGGER ID_system_generator DISABLE;

--- 3. PROJEKT ---
-- 2 dotazy využívající spojení dvou tabulek
-- Které flotily mají lodi typu X-Wing?
SELECT DISTINCT F.*
FROM Flotila F, Lod L
WHERE F.ID_flotila = L.ID_flotila 
    AND L.typ = 'X-Wing';

-- Vyber jedie, kteri maji modry mec a jsou na na Flotile 1
SELECT DISTINCT R.*
FROM Flotila F, Rytir_jedi R
WHERE F.ID_flotila = R.ID_flotila 
    AND R.barva_svetelneho_mece = 'Modra' 
    AND R.ID_flotila = 1;
    
-- 1 využívající spojení tøí tabulek
-- Ktere flotily jsou na orbite planety Endor
SELECT DISTINCT F.*
FROM Flotila F, Planeta P, Ma_na_orbite O
WHERE F.ID_flotila = O.ID_flotila 
    AND P.ID_planeta = O.ID_planeta
    AND P.jmeno = 'Endor';

-- 2 dotazy s klauzulí GROUP BY a agregaèní funkcí
-- Kolik midichlorianu  se nachazi v jednotlivych flotilach?
SELECT ID_flotila, SUM(mnozstvi_midi_chlorianu)
FROM Rytir_jedi
GROUP BY ID_flotila;

-- Prumerne percentualni zastoupeni kysliku v atmosferach planet ruznych planetarnich systemu.
SELECT S.jmeno, AVG(percent_zastoupeni)
FROM Planeta P, Planetarni_system S, Slozeni_atmosfery A
WHERE P.ID_planeta = A.ID_planeta AND P.ID_system = S.ID_system AND A.Nazev = 'Kyslik'
GROUP BY  S.jmeno;

-- 1 dotaz obsahující predikát EXISTS
-- Ktere planety maji v atmosfere pouze vodik.
SELECT P.*
FROM Planeta P, Slozeni_atmosfery S
WHERE P.ID_planeta = S.ID_planeta
    AND S.Nazev = 'Vodik'
    AND NOT EXISTS (SELECT *
                    FROM Slozeni_atmosfery S
                    WHERE P.ID_planeta = S.ID_planeta
                    AND S.Nazev <> 'Vodik'
    );

-- (1 dotaz s predikátem IN s množinou konstantních dat)
-- Kteri jedijove maji modrou nebo zelenou barvu mece
SELECT Jmeno, barva_svetelneho_mece 
FROM Rytir_jedi
WHERE barva_svetelneho_mece IN ('Modra', 'Zelena');


-- 1 dotaz s predikátem IN s vnoøeným selectem (nikoliv IN s množinou konstantních dat)
-- Na kterych flotilach se nachazi jediove narozeni po roce 2050?
SELECT *
FROM Flotila
WHERE ID_flotila IN (SELECT ID_flotila 
                     FROM Rytir_jedi
                     WHERE datum_narozeni > to_date('01-01-2050','dd-mm-yyyy'));
                     

---------------------------------------------------       
-- 4. PROJEKT --

----------- 2 netrivialni databazove triggery -----------
-- 1. -> automaticke generovani ID - viz vyse ve forme ID_xxxxx_generator
-- viz:
--CREATE SEQUENCE planeta_seq;
--CREATE OR REPLACE TRIGGER ID_planeta_generator
--    BEFORE INSERT ON Planeta
--    FOR EACH ROW
--    WHEN (NEW.ID_planeta IS NULL)
--BEGIN
--    SELECT planeta_seq.NEXTVAL
--    INTO :NEW.ID_planeta
--    FROM DUAL;
--END; 
--/

-- 2. -> kdyz se jediovi rozbije lod je mu prirazena 0
CREATE OR REPLACE TRIGGER znicena_lod
AFTER DELETE ON Lod
FOR EACH ROW
BEGIN
  UPDATE Rytir_jedi
  SET ID_lod = NULL
  WHERE ID_lod = :OLD.ID_lod;
END;
/

-- testovani (select je urcten pro kontrolu pred a po) 
--SELECT * FROM Rytir_jedi;
--SELECT * FROM Lod;
DELETE FROM Lod WHERE ID_lod = 1;
--SELECT * FROM Rytir_jedi;
--SELECT * FROM Lod;
 
----------- 2 Procedury -----------
-- Kontrola spravneho pojmenovani flotily
CREATE OR REPLACE PROCEDURE flotila_spatny_nazev AS
    CURSOR flotily IS SELECT * FROM Flotila;
    pocet_chybnych INTEGER;
    flot flotily%ROWTYPE;
    BEGIN
        pocet_chybnych := 0;
        dbms_output.put_line('Flotily s chybnym nazvem:');
    OPEN flotily;
    LOOP
      FETCH flotily INTO flot;
      EXIT WHEN flotily%NOTFOUND;
      --IF flot.nazev NOT LIKE '%\_%' THEN
      IF NOT REGEXP_LIKE(flot.nazev, '^[A-Z]{3}_\d{3}$') THEN
        dbms_output.put_line('ID_flotily: ' || flot.ID_flotila || ', Nazev: ' || flot.nazev);
        pocet_chybnych := pocet_chybnych + 1;
      END IF;
    END LOOP;
    dbms_output.put_line('Celkem: ' || pocet_chybnych);
    CLOSE flotily;
EXCEPTION
    WHEN NO_DATA_FOUND THEN
        RAISE_APPLICATION_ERROR(-11111, 'V databazi se nenachazi zadna flotila');
    WHEN OTHERS THEN
        RAISE_APPLICATION_ERROR(-22222, 'Interni error');
  END;
/

execute flotila_spatny_nazev();

-- Pocet midichlorianu na zadane flotile
CREATE OR REPLACE PROCEDURE pocet_midich_flotila (zkoumana flotila.ID_flotila%TYPE) AS
    CURSOR rytiri IS SELECT * FROM Rytir_jedi;
    pocet_midich INTEGER;
    jedi rytiri%ROWTYPE;
    BEGIN
        pocet_midich := 0;
    OPEN rytiri;
    LOOP
      FETCH rytiri INTO jedi;
      EXIT WHEN rytiri%NOTFOUND;
      IF jedi.ID_flotila LIKE zkoumana THEN
        pocet_midich := pocet_midich + jedi.mnozstvi_midi_chlorianu;
      END IF;
    END LOOP;
    dbms_output.put_line('Celkovy pocet midichlorianu na flotile s id '|| zkoumana || ' je: ' || pocet_midich);
    CLOSE rytiri;
EXCEPTION
    WHEN NO_DATA_FOUND THEN
        RAISE_APPLICATION_ERROR(-11111, 'V databazi se nenachazi zadni rytiri');
    WHEN OTHERS THEN
        RAISE_APPLICATION_ERROR(-22222, 'Interni error');
  END;
/

execute pocet_midich_flotila('1');

------------- Index + Explain plan -------------
CREATE INDEX planeta_sys_id ON Planeta (ID_SYSTEM);

-- Vypis planetarniho systemu a spocitani vsech jeho obyvatel
EXPLAIN PLAN FOR
    SELECT S.jmeno, SUM(pocet_obyvatel) 
        FROM Planetarni_system S, Planeta P
    WHERE S.ID_system = P.ID_system
    GROUP BY S.jmeno;
SELECT * FROM table (DBMS_XPLAN.DISPLAY);

DROP INDEX planeta_sys_id; 

------------- Pristupova prava pro xebert00 -----------
-- Pristupy k tabulkam
GRANT ALL PRIVILEGES ON Hvezda TO xebert00;
GRANT ALL PRIVILEGES ON Planetarni_system TO xebert00;
GRANT ALL PRIVILEGES ON Planeta TO xebert00;
GRANT ALL PRIVILEGES ON Slozeni_atmosfery TO xebert00;
GRANT ALL PRIVILEGES ON Prvek TO xebert00;
GRANT ALL PRIVILEGES ON Tvori TO xebert00;
GRANT ALL PRIVILEGES ON Ma_na_orbite TO xebert00;
GRANT ALL PRIVILEGES ON Flotila TO xebert00;
GRANT ALL PRIVILEGES ON Lod TO xebert00;
GRANT ALL PRIVILEGES ON Rytir_Jedi TO xebert00;
GRANT ALL PRIVILEGES ON Nakladni TO xebert00;
GRANT ALL PRIVILEGES ON Utocna TO xebert00;
GRANT ALL PRIVILEGES ON Velitelska TO xebert00;

-- Pristupy k proceduram
GRANT EXECUTE ON flotila_spatny_nazev TO xebert00;
GRANT EXECUTE ON pocet_midich_flotila TO xebert00;

------------- Materializovany pohled -----------
-- Vypis jediu s jejich lodemi
CREATE MATERIALIZED VIEW rytiri_a_lode
    CACHE
    BUILD IMMEDIATE
    REFRESH COMPLETE
    ENABLE QUERY REWRITE
AS SELECT R.jmeno, R.mnozstvi_midi_chlorianu, L.* 
    FROM xcepel03.Rytir_jedi R, xcepel03.Lod L 
    WHERE R.ID_LOD = L.ID_LOD;

GRANT ALL ON rytiri_a_lode TO xebert00;


SELECT * FROM rytiri_a_lode;        
-- Vlozeni novych prvku
INSERT INTO xcepel03.Lod 
    VALUES (1,'Razor',to_date('21-03-2011','dd-mm-yyyy'),3);
INSERT INTO xcepel03.Rytir_jedi 
    VALUES (22,'Mandalorian',33,'Cervena',to_date('15-12-2110','dd-mm-yyyy'),1,1,1);
-- Zobrazeni pred aktualizaci
SELECT * FROM rytiri_a_lode;
-- Aktualizace + zobrazeni po aktualizaci
EXECUTE DBMS_SNAPSHOT.REFRESH('rytiri_a_lode', 'COMPLETE');
SELECT * FROM rytiri_a_lode;

------------- Komplexni dotaz -------------
-- Vyhodnoti silu flotily podle poctu midichlorianu na ni.
WITH midi_total AS (
    SELECT 
        ID_flotila, 
        SUM(mnozstvi_midi_chlorianu) AS midi_chloriany
    FROM Rytir_jedi
    GROUP BY ID_flotila
)
SELECT 
  ID_flotila, 
  CASE 
    WHEN midi_chloriany > 160 THEN 'Silna flotila'
    WHEN midi_chloriany > 80 THEN 'Prumerna flotila'
    ELSE 'Slaba flotila'
  END AS sila_flotily
FROM midi_total;