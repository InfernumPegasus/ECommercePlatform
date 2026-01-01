#!/usr/bin/env sh

docker exec postgres pg_dump -U ecommerce -d ecommerce --schema-only --schema=public > all_tables.sql