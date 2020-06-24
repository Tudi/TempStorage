from peewee import SqliteDatabase, Model, CharField, DateField, ForeignKeyField, IntegerField

dbTVShows = SqliteDatabase('TVShows.db')


class TVShow(Model):
    class Meta:
        database = dbTVShows

    name = CharField()
    year_start = DateField()
    year_end = DateField()


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


if __name__ == '__main__':
    create_tables()
    drop_tables()
