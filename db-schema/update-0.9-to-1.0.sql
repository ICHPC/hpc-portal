-- Updates a pre-release database to a version 1.0 one.
-- 1. Deleting a project is now allowed.
ALTER TABLE job_list DROP CONSTRAINT "$3" , ADD CONSTRAINT project_id FOREIGN KEY (project_id) REFERENCES projects(project_id) ON DELETE CASCADE ON UPDATE CASCADE;

-- 2. Drops unneeded table.
DROP TABLE foo;

-- 3. Replaces userid with user_id throughout for consistency.
ALTER TABLE users RENAME COLUMN userid TO user_id;
ALTER TABLE job_list RENAME COLUMN userid TO user_id;
ALTER TABLE profile RENAME COLUMN userid TO user_id;
ALTER TABLE projects RENAME COLUMN userid TO user_id;

-- 4. Moves orcid from users table to profile table.
ALTER TABLE profile ADD COLUMN orcid CHARACTER VARYING(80);
UPDATE profile SET orcid = users.orcid FROM users WHERE profile.user_id = users.user_id;
ALTER TABLE users DROP COLUMN orcid;

-- 5. Removes unneeded columns.
ALTER TABLE users DROP COLUMN figsharekey;
ALTER TABLE users DROP COLUMN figsharesecret;

-- 6. Allow pools to be deleted.
ALTER TABLE pools ADD COLUMN deleted BOOLEAN DEFAULT FALSE NOT NULL;

