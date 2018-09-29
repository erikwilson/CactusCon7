build: FORCE
	docker build -t cactuscon7:latest .

run:
	docker logs -f `docker run -p 5000:5000 --link cc7-redis:redis -d  cactuscon7`

redis:
	docker run -p 6379:6379 -v `pwd`/cc7-redis:/data --name cc7-redis -d redis redis-server --appendonly yes

FORCE: