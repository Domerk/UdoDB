PRAGMA foreign_keys = ON;

DROP TABLE IF EXISTS Учащиеся;
DROP TABLE IF EXISTS Преподаватели;
DROP TABLE IF EXISTS Направленности;
DROP TABLE IF EXISTS Объединения;
DROP TABLE IF EXISTS Группы;
DROP TABLE IF EXISTS Нагрузка;

CREATE TABLE Учащиеся (
-- Поля
"ID"      			INTEGER			PRIMARY KEY			AUTOINCREMENT,
"Фамилия"			VARCHAR(60)		NOT NULL,
"Имя"				VARCHAR(60)		NOT NULL,
"Отчество"		    VARCHAR(60),
"Тип документа"	  	VARCHAR(60)		NOT NULL,
"Номер документа"	VARCHAR(60)		NOT NULL,
"Пол"			    VARCHAR(3)		NOT NULL,
"Год рождения"	  	DATE,

"Район школы"		  VARCHAR(60),
"Школа"			      VARCHAR(60),
"Класс"			      VARCHAR(5),

"Родители"		    VARCHAR(200),
"Домашний адрес"	VARCHAR(200),
"Телефон"   			VARCHAR(100),
"e-mail"     			VARCHAR(100),

"Дата заявления"	DATE,
"Форма обучения"	VARCHAR(60),
"Когда выбыл"		DATE,

"С ослабленным здоровьем" 	BOOLEAN,
"Сирота"    			BOOLEAN,
"Инвалид"   			BOOLEAN,
"На учёте в полиции"		BOOLEAN,
"Многодетная семья"	      	BOOLEAN,
"Неполная семья"      		BOOLEAN,
"Малообеспеченная семья"  	BOOLEAN,
"Мигранты"    		BOOLEAN,


"Примечания"  		VARCHAR(200)
);

CREATE TABLE Преподаватели (
-- Поля
"ID" 				INTEGER			PRIMARY KEY			AUTOINCREMENT,
"Фамилия"   		VARCHAR(60)		NOT NULL,
"Имя"     			VARCHAR(60)		NOT NULL,
"Отчество"    		VARCHAR(60),
"Паспорт"			VARCHAR(60)		NOT NULL,
"Отдел"     		VARCHAR(60)
);

CREATE TABLE Направленности (
"ID" 			INTEGER				PRIMARY KEY			AUTOINCREMENT,
"Название направленности"	VARCHAR(60)		NOT NULL
);

CREATE TABLE Объединения (
-- Поля
"ID" 			INTEGER				PRIMARY KEY			AUTOINCREMENT,
"Название"		VARCHAR(60)		NOT NULL,
"Отдел"			  	VARCHAR(60)		NOT NULL,
"Описание"		VARCHAR(300),
"ID Направленности" INTEGER,

FOREIGN KEY ("ID Направленности")	REFERENCES Направленности("ID")
);


CREATE TABLE Группы (
-- Поля
"ID" 				INTEGER			PRIMARY KEY			AUTOINCREMENT,
"ID объединения" 	INTEGER,
"ID преподавателя" 	INTEGER,
"Номер группы"	   	VARCHAR(30),
"Год обучения"		INTEGER,

-- Параметры
FOREIGN KEY ("ID объединения") 		REFERENCES Объединения("ID")
FOREIGN KEY ("ID преподавателя")	REFERENCES Преподаватели("ID")
);

CREATE TABLE Нагрузка (
-- Поля
"ID учащегося"	INTEGER,
"ID группы" 	INTEGER,

-- Параметры
PRIMARY KEY ("ID учащегося", "ID группы"),
FOREIGN KEY ("ID учащегося") 	REFERENCES Учащиеся("ID")
FOREIGN KEY ("ID группы")		REFERENCES Группы("ID")
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

-- Примеры данных

INSERT INTO Учащиеся("ID", "Тип документа", "Номер документа", "Фамилия", "Имя", "Пол") VALUES ("0", "Паспорт", "123", "Иванов", "Иван", "Муж");
INSERT INTO Учащиеся("ID", "Тип документа", "Номер документа", "Фамилия", "Имя", "Пол") VALUES ("1", "Паспорт", "124", "Топчий", "Алексей", "Муж");
INSERT INTO Учащиеся("ID", "Тип документа", "Номер документа", "Фамилия", "Имя", "Пол") VALUES ("2", "Паспорт", "125", "Герцев", "Алексей", "Муж");
INSERT INTO Учащиеся("ID", "Тип документа", "Номер документа", "Фамилия", "Имя", "Пол") VALUES ("3", "Паспорт", "126", "Гусманова", "Мунира", "Жен");
INSERT INTO Учащиеся("ID", "Тип документа", "Номер документа", "Фамилия", "Имя", "Пол") VALUES ("4", "Свидетельство о рождении", "111", "Петрова", "Мария", "Жен");

INSERT INTO Преподаватели("ID", "Паспорт", "Фамилия", "Имя") VALUES ("0", "200", "Аксёнов", "Алексей");
INSERT INTO Преподаватели("ID", "Паспорт", "Фамилия", "Имя") VALUES ("1", "201", "Сухов", "Роман");
INSERT INTO Преподаватели("ID", "Паспорт", "Фамилия", "Имя") VALUES ("3", "202", "Каяндер", "Ася");
INSERT INTO Преподаватели("ID", "Паспорт", "Фамилия", "Имя") VALUES ("4", "203", "Горелик", "Денис");
