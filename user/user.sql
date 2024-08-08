create table if not exists user 
(id             integer     primary key  autoincrement  not null, 
 account        char(20)    not null, 
 username       char(20)    not null, 
 password       char(20));
