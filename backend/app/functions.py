#!/usr/bin/python3


#################################################################################################################################################
#                                                    CLASSES CONTAINING ALL THE APP FUNCTIONS                                                   #
#################################################################################################################################################


class DB:

    def __init__(self, Config):

        from math import floor
        from os import getcwd
        from os.path import join
        from json import loads, dumps, dump
        from datetime import timedelta, datetime, timezone
        from pymongo import MongoClient, errors, ReturnDocument
        from urllib import parse
        from urllib.request import urlopen
        from bson.objectid import ObjectId

        self.Config         = Config
        self.getcwd         = getcwd
        self.join           = join
        self.floor          = floor
        self.loads          = loads
        self.dumps          = dumps
        self.dump           = dump
        self.datetime       = datetime
        self.ObjectId       = ObjectId
        self.server         = Config.DB_SERVER
        self.port           = Config.DB_PORT
        self.username       = parse.quote_plus(Config.DB_USERNAME)
        self.password       = parse.quote_plus(Config.DB_PASSWORD)
        self.remoteMongo    = MongoClient
        self.ReturnDocument = ReturnDocument
        self.PyMongoError   = errors.PyMongoError
        self.BulkWriteError = errors.BulkWriteError
        self.tls            = False  # MUST SET TO TRUE IN PRODUCTION

    def __del__(self):
        pass


    ####################
    # LAB 2 DATABASE UTIL FUNCTIONS  #
    ####################

    def addUpdate(self, data):
        '''ADD A NEW SENSOR READING TO THE WEATHER COLLECTION'''
        try:
            remotedb    = self.remoteMongo('mongodb://%s:%s@%s:%s' % (self.username, self.password, self.server, self.port), tls=self.tls)
            result      = remotedb.ELET2415.weather.insert_one(data)
        except Exception as e:
            msg = str(e)
            if "duplicate" not in msg:
                print("addUpdate error ", msg)
            return False
        else:
            return True

    def getAllInRange(self, start, end):
        '''RETURNS A LIST OF OBJECTS THAT FALLS WITHIN THE START AND END DATE RANGE'''
        try:
            remotedb    = self.remoteMongo('mongodb://%s:%s@%s:%s' % (self.username, self.password, self.server, self.port), tls=self.tls)
            result      = list(remotedb.ELET2415.weather.find({"timestamp": {"$gte": int(start), "$lte": int(end)}}, {"_id": 0}).sort("timestamp", 1))
        except Exception as e:
            msg = str(e)
            print("getAllInRange error ", msg)
        else:
            return result

    def humidityMMAR(self, start, end):
        '''RETURNS MIN, MAX, AVG AND RANGE FOR HUMIDITY WITHIN THE START AND END DATE RANGE'''
        try:
            remotedb    = self.remoteMongo('mongodb://%s:%s@%s:%s' % (self.username, self.password, self.server, self.port), tls=self.tls)
            result      = list(remotedb.ELET2415.weather.aggregate([{"$match": {"timestamp": {"$gte": int(start), "$lte": int(end)}}}, {"$group": {"_id": None, "humidity": {"$push": "$$ROOT.humidity"}}}, {"$project": {"_id": 0, "max": {"$max": "$humidity"}, "min": {"$min": "$humidity"}, "avg": {"$avg": "$humidity"}, "range": {"$subtract": [{"$max": "$humidity"}, {"$min": "$humidity"}]}}}]))
        except Exception as e:
            msg = str(e)
            print("humidityMMAR error ", msg)
        else:
            return result

    def temperatureMMAR(self, start, end):
        '''RETURNS MIN, MAX, AVG AND RANGE FOR TEMPERATURE WITHIN THE START AND END DATE RANGE'''
        try:
            remotedb    = self.remoteMongo('mongodb://%s:%s@%s:%s' % (self.username, self.password, self.server, self.port), tls=self.tls)
            result      = list(remotedb.ELET2415.weather.aggregate([{"$match": {"timestamp": {"$gte": int(start), "$lte": int(end)}}}, {"$group": {"_id": None, "temperature": {"$push": "$$ROOT.temperature"}}}, {"$project": {"_id": 0, "max": {"$max": "$temperature"}, "min": {"$min": "$temperature"}, "avg": {"$avg": "$temperature"}, "range": {"$subtract": [{"$max": "$temperature"}, {"$min": "$temperature"}]}}}]))
        except Exception as e:
            msg = str(e)
            print("temperatureMMAR error ", msg)
        else:
            return result

    def heatindexMMAR(self, start, end):
        '''RETURNS MIN, MAX, AVG AND RANGE FOR HEAT INDEX WITHIN THE START AND END DATE RANGE'''
        try:
            remotedb    = self.remoteMongo('mongodb://%s:%s@%s:%s' % (self.username, self.password, self.server, self.port), tls=self.tls)
            result      = list(remotedb.ELET2415.weather.aggregate([{"$match": {"timestamp": {"$gte": int(start), "$lte": int(end)}}}, {"$group": {"_id": None, "heatindex": {"$push": "$$ROOT.heatindex"}}}, {"$project": {"_id": 0, "max": {"$max": "$heatindex"}, "min": {"$min": "$heatindex"}, "avg": {"$avg": "$heatindex"}, "range": {"$subtract": [{"$max": "$heatindex"}, {"$min": "$heatindex"}]}}}]))
        except Exception as e:
            msg = str(e)
            print("heatindexMMAR error ", msg)
        else:
            return result

    def pressureMMAR(self, start, end):
        '''RETURNS MIN, MAX, AVG AND RANGE FOR PRESSURE WITHIN THE START AND END DATE RANGE'''
        try:
            remotedb    = self.remoteMongo('mongodb://%s:%s@%s:%s' % (self.username, self.password, self.server, self.port), tls=self.tls)
            result      = list(remotedb.ELET2415.weather.aggregate([{"$match": {"timestamp": {"$gte": int(start), "$lte": int(end)}}}, {"$group": {"_id": None, "pressure": {"$push": "$$ROOT.pressure"}}}, {"$project": {"_id": 0, "max": {"$max": "$pressure"}, "min": {"$min": "$pressure"}, "avg": {"$avg": "$pressure"}, "range": {"$subtract": [{"$max": "$pressure"}, {"$min": "$pressure"}]}}}]))
        except Exception as e:
            msg = str(e)
            print("pressureMMAR error ", msg)
        else:
            return result

    def altitudeMMAR(self, start, end):
        '''RETURNS MIN, MAX, AVG AND RANGE FOR ALTITUDE WITHIN THE START AND END DATE RANGE'''
        try:
            remotedb    = self.remoteMongo('mongodb://%s:%s@%s:%s' % (self.username, self.password, self.server, self.port), tls=self.tls)
            result      = list(remotedb.ELET2415.weather.aggregate([{"$match": {"timestamp": {"$gte": int(start), "$lte": int(end)}}}, {"$group": {"_id": None, "altitude": {"$push": "$$ROOT.altitude"}}}, {"$project": {"_id": 0, "max": {"$max": "$altitude"}, "min": {"$min": "$altitude"}, "avg": {"$avg": "$altitude"}, "range": {"$subtract": [{"$max": "$altitude"}, {"$min": "$altitude"}]}}}]))
        except Exception as e:
            msg = str(e)
            print("altitudeMMAR error ", msg)
        else:
            return result

    def soilMMAR(self, start, end):
        '''RETURNS MIN, MAX, AVG AND RANGE FOR SOIL MOISTURE WITHIN THE START AND END DATE RANGE'''
        try:
            remotedb    = self.remoteMongo('mongodb://%s:%s@%s:%s' % (self.username, self.password, self.server, self.port), tls=self.tls)
            result      = list(remotedb.ELET2415.weather.aggregate([{"$match": {"timestamp": {"$gte": int(start), "$lte": int(end)}}}, {"$group": {"_id": None, "soil": {"$push": "$$ROOT.soil"}}}, {"$project": {"_id": 0, "max": {"$max": "$soil"}, "min": {"$min": "$soil"}, "avg": {"$avg": "$soil"}, "range": {"$subtract": [{"$max": "$soil"}, {"$min": "$soil"}]}}}]))
        except Exception as e:
            msg = str(e)
            print("soilMMAR error ", msg)
        else:
            return result

    def frequencyDistro(self, variable, start, end):
        '''RETURNS THE FREQUENCY DISTRIBUTION FOR A SPECIFIED VARIABLE WITHIN THE START AND END DATE RANGE'''
        try:
            remotedb    = self.remoteMongo('mongodb://%s:%s@%s:%s' % (self.username, self.password, self.server, self.port), tls=self.tls)
            result      = list(remotedb.ELET2415.weather.aggregate([{"$match": {"timestamp": {"$gte": int(start), "$lte": int(end)}}}, {"$bucket": {"groupBy": "$" + variable, "boundaries": [0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100], "default": "outliers", "output": {"count": {"$sum": 1}}}}]))
        except Exception as e:
            msg = str(e)
            print("frequencyDistro error ", msg)
        else:
            return result


def main():
    from config import Config
    from time import time, ctime, sleep
    from math import floor
    from datetime import datetime, timedelta
    one = DB(Config)

    start = time()
    end   = time()
    print(f"completed in: {end - start} seconds")

if __name__ == '__main__':
    main()