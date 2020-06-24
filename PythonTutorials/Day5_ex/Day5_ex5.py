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

    tv_show = ForeignKeyField(TVShow, backref='episodes', null=True)
    name = CharField(null=True)
    season = IntegerField(default=1, null=True)
    air_date = DateField(default=datetime.date.today(), null=True)


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


def AddNewEpisodeEntry(EpisodeName, TVShowId):
    eps = Episode()
    eps.name = EpisodeName
    eps.tv_show = TVShowId
    eps.save()
    #    print(eps)
    return {'id': eps.get_id(), 'name': eps.name, 'tv_show': eps.tv_show, 'season': eps.season,
            "air_date": eps.air_date}


def SelectEpisodes(StartDate, EndDate):
    query = Episode().select().filter(Episode.air_date >= str(StartDate)).filter(Episode.air_date <= str(EndDate))
    #print(query)
    res = query.get()
    print(res.name)


if __name__ == '__main__':
    create_tables()
    ShowId = AddNewTVSEntry("MyShow")  # need to create this to be able to get a valid ID

    AddNewEpisodeEntry("ep1", ShowId)
    AddNewEpisodeEntry("ep2", ShowId)

    SelectEpisodes(datetime.date.today(), datetime.date.today())

    drop_tables()
