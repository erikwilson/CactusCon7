# Cactus Coin API
The Cactus Coin API is a REST API designed to provide badges with an interface to register, and submit coins. It also provides the front end with an interface to query data regarding earned badge coins, badge names, locations, an overall leaderboard.


## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. See deployment for notes on how to deploy the project on a live production system.

### Prerequisites

What things you need to install the software and how to install them

```
Give examples
```

### Installing

The Cactuscon API is packaged with setuptools and can be installed using the standard process for such packages.
```
python setup.py install
```

It is then possible to start and run the application in development mode.

```
export FLASK_APP=cactuscoinapi
export FLASK_DEBUG=true
flask run
```

To deploy the application into a production environment, see the deployment section of the wiki.

## Running the tests

This project uses pytest to execute unit tests.  Running tests is simple, just run:

```
python setup.py test
```

## Deployment

Add additional notes about how to deploy this on a live system

## Built With

* [Flask](http://flask.pocoo.org/) - The web framework used
* [Flask-RESTful](https://flask-restful.readthedocs.io/) - REST extensions for flask
* [Redis](https://redislabs.com/lp/python-redis/) - Backend data structure storage

## Versioning

We use [SemVer](http://semver.org/) for versioning. For the versions available, see the [tags on this repository](https://github.com/erikwilson/CactusCon7). 

## Authors

* **cybaix** - *Initial work* - [cybaix](https://github.com/cybaix)

See also the list of [contributors](https://github.com/your/project/contributors) who participated in this project.

## License


## Acknowledgments


