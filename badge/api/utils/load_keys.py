""" Utility to load public keys (PEM) into redis for use with the cactuscoinapi.
"""
import sys
import argparse
import logging
import redis
import pathlib

LOGGING_FORMAT = "%(message)s"

def main(argv=None):
    if argv is None:
        argv = sys.argv[1:]

    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--redis-server", '-H', default='localhost', help='Redis server to connect to.')
    p.add_argument("--redis-port", '-p', default=6379, help='Port to use with redis server.')
    p.add_argument("pubkeyfile", nargs="+", help='')

    args = p.parse_args(argv)
    fmt = LOGGING_FORMAT

    """
    if args.verbose == 1:
        logging.basicConfig(level=logging.INFO, format=fmt)
    elif args.verbose == 2:
        logging.basicConfig(level=logging.DEBUG, format=fmt)
    else:
        logging.basicConfig(level=logging.WARNING, format=fmt)
    """

    logger = logging.getLogger()

    r = redis.Redis(host=args.redis_server, port=args.redis_port)
    
    for keyfilepath in args.pubkeyfile:
        badge_num = int(pathlib.Path(keyfilepath).name)
        with open(keyfilepath, 'r') as key_file:
            r.hset('badge_keys', badge_num, key_file.read())

    return 0

if __name__ == "__main__":
    sys.exit(main())

