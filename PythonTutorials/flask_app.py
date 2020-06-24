import datetime

from flask import Flask, request, render_template
from peewee import Model, CharField, DateField, SqliteDatabase

app = Flask(__name__)
app.config['DEBUG'] = True
app.config["Environment"] = "Development"


@app.route('/index')
def Hello_world():
    return "Hello world"


@app.route('/test/<int:first_number>/<int:second_number>')
def Hello_world2(first_number=0, second_number=0):
    return f"Hello world. Got numbers {first_number} {second_number}"


@app.route('/<first_number>/<second_number>')
def Hello_world3(first_number=0, second_number=0):
    return f" Got numbers {first_number} {second_number}, {request.args}"


dbTVShows = SqliteDatabase('TVShows.db')


class TVShow(Model):
    class Meta:
        database = dbTVShows

    name = CharField()
    year_start = DateField(default=datetime.date.today(), null=True)
    year_end = DateField(default=datetime.date.today(), null=True)


def create_tables():
    dbTVShows.connect()
    dbTVShows.create_tables([TVShow])


def drop_tables():
    dbTVShows.drop_tables([TVShow])


def AddNewTVSEntry(TVShowName):
    tvs = TVShow()
    tvs.name = TVShowName
    tvs.save()
    return tvs.get_id()


@app.route('/tv-show/<int:tv_show_id>')
def tv_show_details(tv_show_id):
    toPrint = f"TV show {tv_show_id}"
    # QueryRes = TVShow().select().filter(id=tv_show_id).get()
    try:
        QueryRes = TVShow().get_by_id(tv_show_id)
        toPrint += "<br>Name of the show : " + QueryRes.name
    except TVShow.DoesNotExist as ex:
        toPrint += "<br>This id has no show assigned to it. Err : " + str(ex)
    return toPrint


@app.route('/tvshow/<int:tv_show_id>')
def tv_show_details_templated(tv_show_id):
    try:
        tv_show = TVShow().get_by_id(tv_show_id)
        return render_template("tv_show_template.html", tv_show=tv_show)
    except TVShow.DoesNotExist as ex:
        return "Id does not exist"
    return ""


create_tables()
AddNewTVSEntry("MyShow")
app.run()
