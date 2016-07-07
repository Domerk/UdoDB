﻿PRAGMA foreign_keys = ON;

DROP TABLE IF EXISTS Учащиеся;
DROP TABLE IF EXISTS Преподаватели;
DROP TABLE IF EXISTS Направленности;
DROP TABLE IF EXISTS Объединение;
DROP TABLE IF EXISTS Группа;
DROP TABLE IF EXISTS Нагрузка;

CREATE TABLE Учащиеся (
-- Поля
`ID`      			INTEGER			PRIMARY KEY			AUTOINCREMENT,
`Фамилия`			VARCHAR(100)		NOT NULL,
`Имя`				VARCHAR(100)		NOT NULL,
`Отчество`		    VARCHAR(100),
`Тип документа`	  	VARCHAR(60)		NOT NULL,
`Номер документа`	VARCHAR(60)		NOT NULL,
`Пол`			    VARCHAR(3)		NOT NULL,
`Дата рождения`	  	DATE,

`Район школы`		  VARCHAR(60),
`Школа`			      VARCHAR(60),
`Класс`			      VARCHAR(5),

`Родители`		    VARCHAR(300),
`Домашний адрес`	VARCHAR(300),
`Телефон`   			VARCHAR(100),
`e-mail`     			VARCHAR(100),

`Дата заявления`	DATE,
`Форма обучения`	VARCHAR(60),
`Когда выбыл`		DATE,

`С ослабленным здоровьем` 	BOOLEAN,
`Сирота`    			BOOLEAN,
`Инвалид`   			BOOLEAN,
`На учёте в полиции`		BOOLEAN,
`Многодетная семья`	      	BOOLEAN,
`Неполная семья`      		BOOLEAN,
`Малообеспеченная семья`  	BOOLEAN,
`Мигранты`    		BOOLEAN,


`Примечания`  		VARCHAR(500)
);

CREATE TABLE Преподаватели (
`ID` 				INTEGER			PRIMARY KEY			AUTOINCREMENT,
`Фамилия`   		VARCHAR(100)		NOT NULL,
`Имя`     			VARCHAR(100)		NOT NULL,
`Отчество`    		VARCHAR(100),
`Паспорт`			VARCHAR(60)		NOT NULL,
`Отдел`     		VARCHAR(60)
);

CREATE TABLE Направленности (
`ID` 			INTEGER				PRIMARY KEY			AUTOINCREMENT,
`Название`	VARCHAR(60)		NOT NULL
);

CREATE TABLE Объединение (
`ID` 			INTEGER				PRIMARY KEY			AUTOINCREMENT,
`Название`		VARCHAR(100)		NOT NULL,
`Отдел`			VARCHAR(60),
`Описание`		VARCHAR(500),
`ID направленности` INTEGER,

FOREIGN KEY (`ID направленности`)	REFERENCES Направленности(`ID`)
);

CREATE TABLE Группа (
`ID` 				INTEGER			PRIMARY KEY			AUTOINCREMENT,
`ID объединения` 	INTEGER,
`ID преподавателя` 	INTEGER,
`Номер`			   	VARCHAR(30),
`Год обучения`		INTEGER,

FOREIGN KEY (`ID объединения`) 		REFERENCES Объединение(`ID`)		ON DELETE CASCADE,
FOREIGN KEY (`ID преподавателя`)	REFERENCES Преподаватели(`ID`)		ON DELETE CASCADE
);

CREATE TABLE Нагрузка (
`ID учащегося`	INTEGER,
`ID группы` 	INTEGER,

PRIMARY KEY (`ID учащегося`, `ID группы`),
FOREIGN KEY (`ID учащегося`) 	REFERENCES Учащиеся(`ID`) 	ON DELETE CASCADE,
FOREIGN KEY (`ID группы`)		REFERENCES Группа(`ID`)		ON DELETE CASCADE
);


-- Предстваления

DROP VIEW IF EXISTS Объединения;
CREATE VIEW Объединения AS
	SELECT Объединение.`ID` `ID`, Направленности.`ID` `ID направленности`, Объединение.`Название` `Название`, Направленности.`Название` `Направленность`, Объединение.`Отдел` `Отдел`,  Объединение.`Описание` `Описание`
	FROM Объединение, Направленности
	WHERE Объединение.`ID направленности` = Направленности.`ID`
;

DROP VIEW IF EXISTS Группы;
CREATE VIEW Группы AS 
	SELECT Группа.`ID` `ID`, Группа.`ID преподавателя` `ID преподавателя`, Группа.`Номер` `Номер`, Группа.`Год обучения` `Год обучения`, Объединение.`ID` `ID объединения`, Объединение.`Название` `Объединение`, Преподаватели.`Фамилия` `Фамилия преподавателя`, Преподаватели.`Имя` `Имя преподавателя`, Преподаватели.`Отчество` `Отчество преподавателя`
	FROM Группа, Объединение, Преподаватели
	WHERE Группа.`ID объединения` = Объединение.`ID`
	AND Группа.`ID преподавателя` = Преподаватели.`ID`
;

DROP VIEW IF EXISTS Состав_групп;
CREATE VIEW Состав_групп AS
	SELECT Группа.`ID` `ID группы`, Учащиеся.`ID` `ID учащегося`, Группа.`Номер` `Номер группы`, Учащиеся.`Фамилия` `Фамилия`, Учащиеся.`Имя` `Имя`, Учащиеся.`Отчество` `Отчество`, Учащиеся.`Телефон` `Телефон`, Учащиеся.`e-mail` `e-mail`
	FROM Группа, Нагрузка, Учащиеся
	WHERE Группа.`ID` = Нагрузка.`ID группы`
	AND Учащиеся.`ID` = Нагрузка.`ID учащегося`
;

DROP VIEW IF EXISTS Нагрузка_Учащегося;
CREATE VIEW Нагрузка_Учащегося AS
	SELECT Объединение.`ID` `ID объединения`, Группа.`ID` `ID группы`, Объединение.`Название` `Объединение`, Группа.`Номер` `Номер группы`, Нагрузка.`ID учащегося` `ID учащегося`
	FROM Группа, Нагрузка, Объединение
	WHERE Группа.`ID` = Нагрузка.`ID группы`
	AND Группа.`ID объединения` = Объединение.`ID`;

-- Триггеры

DROP TRIGGER IF EXISTS AllInsertTrigger;
CREATE TRIGGER AllInsertTrigger
 AFTER INSERT ON Объединение
 BEGIN 
	INSERT INTO Группа (`Номер`,`ID объединения`) VALUES ("Без группы", (SELECT `ID` FROM Объединение WHERE `Название` = NEW.`Название`));
 END;
  
 DROP TRIGGER IF EXISTS AllDeleteTrigger;
 CREATE TRIGGER AllDeleteTrigger
 INSTEAD OF DELETE ON Объединения
 BEGIN 
	DELETE FROM Объединение WHERE `ID` = OLD.`ID`;
 END;
 
 DROP TRIGGER IF EXISTS GroupDeleteTrigger;
 CREATE TRIGGER GroupDeleteTrigger
 INSTEAD OF DELETE ON Группы
 BEGIN 
	DELETE FROM Группа WHERE `ID` = OLD.`ID`;
 END;
 
 -- Удалить запись в таблице Нагрузка о том, что учащийся в группе "Без группы"
 -- при добавлении новой, если у них одинаковые учащиеся и объединения
DROP TRIGGER IF EXISTS StudInGroupTrigger;
CREATE TRIGGER StudInGroupTrigger
AFTER INSERT ON Нагрузка
BEGIN 
	DELETE FROM Нагрузка WHERE `ID учащегося` = NEW.`ID учащегося`
	AND `ID группы` = (SELECT `ID группы` FROM Нагрузка_Учащегося WHERE `ID учащегося` = NEW.`ID учащегося` AND `Номер группы` = "Без группы" AND `ID объединения` = (SELECT `ID объединения` FROM Нагрузка_Учащегося WHERE `ID учащегося` = NEW.`ID учащегося` AND `ID группы` = NEW.`ID группы`));
 END;
