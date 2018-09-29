FROM ubuntu:18.10

RUN apt-get update -y && \
    apt-get install -y python3 python3-pip python3-dev

COPY . /app
WORKDIR /app

RUN pip3 install flask
RUN python3 setup.py install

ENV LC_ALL C.UTF-8
ENV LANG C.UTF-8

ENV CACTUSCONAPI_SETTINGS_FILE cactuscoinapi.cfg
ENV FLASK_APP cactuscoinapi

ENTRYPOINT [ "flask" ]
CMD [ "run", "-h", "0.0.0.0" ]