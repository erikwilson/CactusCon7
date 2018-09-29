from setuptools import setup

setup(
    name='cactuscoinapi',
    packages=['cactuscoinapi'],
    include_package_data=False,
    install_requires=[
        'flask',
        'flask-redis',
        'flask-restful',
        'cryptography',
        'flask-cors'
    ],
    setup_requires=[
        'pytest',
        'pytest-flask',
        'pytest-datadir',
        'pytest-runner',
        'pytest-cov',
        'pytest-mock',
        'mockredispy',
    ]
)
