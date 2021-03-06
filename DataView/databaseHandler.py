# This module handles the setup and interface with the database
import sqlite3

database = "development.db"


def create_connection(db_file):
    # create a connection to a sqlite database
    conn = None
    try:
        conn = sqlite3.connect(db_file)
        print(sqlite3.version)
    except Exception as e:
        print(e)
    return conn


def create_table(connection, SQL_QUERY):
    try:
        c = connection.cursor()
        c.execute(SQL_QUERY)
    except Exception as e:
        print(e)


def main():
    sql_create_radios_table = """ CREATE TABLE IF NOT EXISTS radios (
                                    ssid integer PRIMARY_KEY,
                                    short_id text,
                                    calls_id integer,
                                    lastSeen timestamp
                                    channel_nums integer,
                                    FOREIGN KEY (calls_id)
                                        REFERENCES calls(id),
                                    PRIMARY KEY (ssid)
                                );"""

    sql_create_calls_table = """ CREATE TABLE IF NOT EXISTS calls (
                                    id integer PRIMARY_KEY,
                                    isEmergency integer,
                                    start_time timestamp,
                                    end_time timestamp,
                                    radio_id integer,
                                    session_id integer,
                                    FOREIGN KEY (session_id)
                                        REFERENCES sessions(id),
                                    PRIMARY KEY (id)
                                );"""

    sql_create_sessions_table = """ CREATE TABLE IF NOT EXISTS sessions (
                                    id integer PRIMARY_KEY,
                                    calls_count integer,
                                    start_time timestamp,
                                    end_time timestamp,
                                    isEmergency integer,
                                    PRIMARY KEY (id)
                                );"""

    # channels table to be used later for region-matching
    sql_create_channels_table = """ CREATE TABLE IF NOT EXISTS channels (
                                    channel_num integer PRIMARY_KEY,
                                    sessions_id integer,
                                    FOREIGN KEY (sessions_id)
                                        REFERENCES sessions(id),
                                    PRIMARY KEY (channel_num)
                                );"""

    sql_create_radiosCount_table = """ CREATE TABLE IF NOT EXISTS radiosCount (
                                    dateTime timestamp PRIMARY_KEY,
                                    radiosCount integer,
                                    PRIMARY KEY (dateTime)
                                );"""

    conn = create_connection(database)
    if conn is not None:
        create_table(conn, sql_create_radios_table)
        create_table(conn, sql_create_calls_table)
        create_table(conn, sql_create_sessions_table)
        create_table(conn, sql_create_channels_table)
        create_table(conn, sql_create_radiosCount_table)
        conn.close()
    else:
        print("Error, cannot create the database connection")


def addCallToDB(id, isEmergency, start_time, end_time, radio_id, session):
    conn = create_connection(database)
    c = conn.cursor()
    sql = ''' INSERT INTO calls(id,
                                isEmergency,
                                start_time,
                                end_time,
                                radio_id,
                                session_id)
            VALUES(?,?,?,?,?,?) '''
    params = (id, isEmergency, start_time, end_time, radio_id, session)
    c.execute(sql, params)
    conn.commit()

    return c.lastrowid


def addSessionToDB(id, calls_count, start_time, end_time):
    conn = create_connection(database)
    c = conn.cursor()
    sql = ''' INSERT INTO sessions(id,
                                  calls_count,
                                  start_time,
                                  end_time)
            VALUES(?,?,?,?) '''
    params = (id, calls_count, start_time, end_time)
    c.execute(sql, params)
    conn.commit()

    return c.lastrowid


def addRadioToDB(ssid, lastSeen):
    conn = create_connection(database)
    c = conn.cursor()
    sql = ''' INSERT OR IGNORE INTO radios(ssid,
                                           short_id,
                                           lastSeen)
            VALUES(?,?,?) '''
    params = (ssid, str(ssid), lastSeen)
    c.execute(sql, params)
    conn.commit()


def addSimpleCount(dateTime, radiosCount):
    conn = create_connection(database)
    c = conn.cursor()
    sql = ''' INSERT INTO radio_counts(timestamp,
                                 count)
            VALUES(?,?) '''
    params = (dateTime, radiosCount)
    c.execute(sql, params)
    conn.commit()
    return c.lastrowid


def resumeUniqueIDs():
    highestID = 0
    tables = ["calls", "sessions"]
    columns = ["id", "id"]
    conn = create_connection(database)
    c = conn.cursor()
    sql = ''''''

    for i in range(0, 1):
        table = tables[i]
        column = columns[i]
        sql = ''' SELECT '''+column+'''
                    FROM '''+table+'''
                    ORDER BY '''+column+''' DESC
                    LIMIT 1 '''

        row = c.execute(sql)
        if (row is not None):
            highest = row.fetchone()
            if (highest is not None):
                highest = highest[0]
                print(highest)
                if (highest > highestID):
                    highestID = highest

    return (highestID + 1)
