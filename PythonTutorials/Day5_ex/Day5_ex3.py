import datetime

from peewee import SqliteDatabase, Model, CharField, DateField, ForeignKeyField, IntegerField

dbTVShows = SqliteDatabase('TVShows.db')


class TVShow(Model):
    class Meta:
        database = dbTVShows

    name = CharField()
    year_start = DateField(default=datetime.date.today(), null=True)
    year_end = DateField(default=datetime.date.today(), null=True)


class Episode(Model):
    class Meta:
        database = dbTVShows

    tv_show = ForeignKeyField(TVShow, backref='episodes')
    name = CharField()
    season = IntegerField()
    air_date = DateField()


def create_tables():
    dbTVShows.connect()
    dbTVShows.create_tables([TVShow, Episode])


def drop_tables():
    dbTVShows.drop_tables([TVShow, Episode])


def AddNewTVSEntry(TVShowName):
    tvs = TVShow()
    tvs.name = TVShowName
    tvs.save()
    return tvs.get_id()

if __name__ == '__main__':
    create_tables()
    print( AddNewTVSEntry("strangerthings"))
    print( AddNewTVSEntry("got"))
    drop_tables()
