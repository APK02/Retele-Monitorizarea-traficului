DROP TABLE trafic;

CREATE TABLE trafic (
    str_id NUMBER(2) NOT NULL,
    nume_str VARCHAR2(100),
    viteza_max NUMBER(3),
    evenimente TEXT
);

INSERT INTO trafic VALUES (1, 'Mihai Viteazu', 50, NULL);
INSERT INTO trafic VALUES (2, 'Stefan cel Mare', 70, NULL);
INSERT INTO trafic VALUES (3, 'Alexandru I. Cuza', 80, NULL);
INSERT INTO trafic VALUES (4, 'Sportului', 50, NULL);
INSERT INTO trafic VALUES (5, 'Crinilor', 60, NULL);
INSERT INTO trafic VALUES (6, 'Ceferistilor', 70, NULL);
INSERT INTO trafic VALUES (7, '1 Decembrie', 30, NULL);

SELECT * FROM trafic;

DROP TABLE vreme;

CREATE TABLE vreme (
    id_vreme NUMBER(2) NOT NULL,
    zi_saptamana VARCHAR2(100),
    temperatura NUMBER(2),
    temperatura_max NUMBER(2),
    temperatura_min NUMBER(2),
    temp_resimtita NUMBER(2),
    status VARCHAR(2)
);

INSERT INTO vreme VALUES (1, 'Luni', -5, -4, -11, -6, 'Ploaie si zapada');
INSERT INTO vreme VALUES (2, 'Marti', -10, -9, -13, -10, 'Ninsoare');
INSERT INTO vreme VALUES (3, 'Miercuri', -5, -2, -7, -3, 'Cer Inorat');
INSERT INTO vreme VALUES (4, 'Joi', 0, 2, -4, 0, 'Cer Senin');
INSERT INTO vreme VALUES (5, 'Vineri', -5, -1, -7, -3, 'Putin Inorat');
INSERT INTO vreme VALUES (6, 'Sambata', 3, 4, -6, 3, 'Ploaie');
INSERT INTO vreme VALUES (7, 'Duminica', 6, 7, 1, 5, 'Cer Senin');

SELECT * FROM vreme;

DROP TABLE sporturi;

CREATE TABLE sporturi (
    id_eveniment NUMBER(2) NOT NULL,
    detalii_eveniment VARCHAR2(100),
    ora VARCHAR2(100),
    data VARCHAR2(100)
);

INSERT INTO sporturi VALUES (1, 'Raliuri', '09:00', '10/01/24');
INSERT INTO sporturi VALUES (2, 'Meci de fotbal: Poli Iasi - FCSB', '22:00', '12/01/24');
INSERT INTO sporturi VALUES (3, 'Meci de tenis: Roger Federer - Rafael Nadal', '18:30', '20/01/24');
INSERT INTO sporturi VALUES (4, 'Maraton de alergat', '10:00', '30/01/24');
INSERT INTO sporturi VALUES (5, 'Ciclism - Turul Iasului', '10:30', '04/03/24');
INSERT INTO sporturi VALUES (6, 'Meci de fotbal: Real Madrid - Barcelona', '19:30', '25/02/24');
INSERT INTO sporturi VALUES (7, 'Concurs: desene din creta pe asfalt', '11:00', '01/03/24');

SELECT * FROM sporturi;

DROP TABLE peco;

CREATE TABLE peco (
    id_peco NUMBER(2) NOT NULL,
    nume_companie VARCHAR2(100),
    nume_str VARCHAR2(100),
    pret_benzina DOUBLE,
    pret_motorina DOUBLE
);

INSERT INTO peco VALUES (1, 'Petrom', 'Mihai Viteazu', 7.00, 7.50);
INSERT INTO peco VALUES (2, 'OMV', 'Stefan cel Mare', 7.23, 7.73);
INSERT INTO peco VALUES (3, 'Rompetrol', 'Alexandru I. Cuza', 8.00, 8.00);
INSERT INTO peco VALUES (4, 'Lukoil', 'Bulevardul Cantemir', 6.98, 7.00);
INSERT INTO peco VALUES (5, 'MOL', 'Mihail Kogalniceanu', 7.00, 7.50);
INSERT INTO peco VALUES (6, 'Petrom', 'Sportului', 7.00, 7.50);
INSERT INTO peco VALUES (7, 'Rompetrol', 'Culturii', 7.00, 7.50);

SELECT * FROM peco;