create table flag_storage(
    id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
    ip VARCHAR(100) NOT NULL,
    challenge_name VARCHAR(255) NOT NULL,
    flag VARCHAR(255) NOT NULL,
    date DATETIME NOT NULL
);
create table flag_log(
    id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
    date DATETIME NOT NULL,
    flag VARCHAR(255) NOT NULL,
    ip VARCHAR(255) NOT NULL,
    team_name TEXT NOT NULL,
    comment TEXT NOT NULL
);
