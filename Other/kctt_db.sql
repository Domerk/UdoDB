PRAGMA foreign_keys = ON;

DROP TABLE IF EXISTS Учащиеся;
DROP TABLE IF EXISTS Преподаватели;
DROP TABLE IF EXISTS Объединения;
DROP TABLE IF EXISTS Группы;
DROP TABLE IF EXISTS Нагрузка;

CREATE TABLE Учащиеся (
-- Поля
"ID"      				INTEGER,
"Тип документа"	  VARCHAR(60)		NOT NULL,
"Номер документа"	VARCHAR(60)		NOT NULL,
"Фамилия"			    VARCHAR(60)		NOT NULL,
"Имя"				      VARCHAR(60)		NOT NULL,
"Отчество"		    VARCHAR(60),
"Пол"			      	VARCHAR(1)		NOT NULL,
"Год рождения"	  DATE,

"Дата заявления"	DATE,
"Когда выбыл"		  DATE,

"Район школы"		  VARCHAR(60),
"Школа"			      VARCHAR(60),
"Класс"			      VARCHAR(5),

"Родители"		    VARCHAR(200),
"Домашний адрес"	VARCHAR(200),
"Телефон"   			VARCHAR(100),
"e-mail"     			VARCHAR(100),

"Сирота"    			BOOLEAN,
"Инвалид"   			BOOLEAN,
"С ослабленным здоровьем" 	BOOLEAN,
"На учёте в милиции"		  	BOOLEAN,
"Многодетная семья"	      	BOOLEAN,
"Неполная семья"      			BOOLEAN,
"Малообеспеченная семья"  	BOOLEAN,
"Мигранты"    		BOOLEAN,

"Форма обучения"	VARCHAR(60),
"Примечания"  		VARCHAR(200),

-- Параметры
PRIMARY KEY ("ID")
);

CREATE TABLE Преподаватели (
-- Поля
"ID" 				      INTEGER,
"Номер паспорта"	VARCHAR(60)		NOT NULL,
"Фамилия"   			VARCHAR(60)		NOT NULL,
"Имя"     				VARCHAR(60)		NOT NULL,
"Отчество"    		VARCHAR(60),
"Отдел"     			VARCHAR(60),

-- Параметры
PRIMARY KEY ("ID")
);


CREATE TABLE Объединения (
-- Поля
"ID" 				  INTEGER,
"Название"		VARCHAR(60)		NOT NULL,
"Направленность"	VARCHAR(60)		NOT NULL,
"Отдел"			  VARCHAR(60)		NOT NULL,
"Описание"		VARCHAR(300),

-- Параметры
PRIMARY KEY ("ID")
);


CREATE TABLE Группы (
-- Поля
"ID" 				       INTEGER,
"ID объединения" 	 INTEGER,
"ID преподавателя" INTEGER,
"Номер группы"	   VARCHAR(30),

-- Параметры
PRIMARY KEY ("ID"),
FOREIGN KEY ("ID объединения") 	REFERENCES Объединения("ID")
FOREIGN KEY ("ID преподавателя")	REFERENCES Преподаватели("ID")
);


CREATE TABLE Нагрузка (
-- Поля
"ID учащегося"	 INTEGER,
"ID группы" 		 INTEGER,

-- Параметры
PRIMARY KEY ("ID учащегося", "ID группы"),
FOREIGN KEY ("ID учащегося") 	REFERENCES Учащиеся("ID")
FOREIGN KEY ("ID группы")	REFERENCES Группы("ID")
);


-- Предстваления

DROP VIEW IF EXISTS Состав_групп;
DROP VIEW IF EXISTS Преподаватели_групп;


CREATE VIEW Состав_групп AS
	SELECT Группы."ID", Учащиеся."ID", Группы."Номер группы", Учащиеся."Фамилия", Учащиеся."Имя", Учащиеся."Отчество", Учащиеся."Телефон", Учащиеся."e-mail"
	FROM Группы, Нагрузка, Учащиеся
	WHERE Группы."ID" = Нагрузка."ID группы"
	AND Учащиеся."ID" = Нагрузка."ID учащегося"
	GROUP BY (Группы."Номер группы")
;

CREATE VIEW Преподаватели_групп AS
	SELECT Группы."ID", Преподаватели."ID", Группы."Номер группы", Преподаватели."Имя", Преподаватели."Фамилия", Преподаватели."Отчество", Преподаватели."Отдел" 
	FROM Группы, Преподаватели
	WHERE Группы."ID преподавателя" = Преподаватели."ID"
	GROUP BY (Преподаватели."ID")
;