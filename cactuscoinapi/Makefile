build: FORCE
	docker build -t cactuscon7:latest .

run:
	docker logs -f `docker run -p 5000:5000 --link cc7-redis:redis --name cc7 -d cactuscon7`

redis:
	docker run -p 127.0.0.1:6379:6379 -v `pwd`/cc7-redis:/data --name cc7-redis -d redis redis-server --appendonly yes

clean:
	docker rm -f cc7
	docker rm -f cc7-redis

FORCE: