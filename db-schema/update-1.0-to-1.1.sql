-- Updates a version 1.0 database to a version 1.1 one
-- 1. Embargo
ALTER TABLE job_list ADD COLUMN embargo_date DATE NOT NULL DEFAULT '0000-01-01';
ALTER TABLE job_list ADD COLUMN embargo_status INTEGER NOT NULL DEFAULT 0;
ALTER TABLE profile ALTER COLUMN embargo SET DEFAULT 90;
