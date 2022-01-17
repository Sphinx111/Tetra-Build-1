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
                                        REFERENCES calls(id)
                                );"""

    sql_create_calls_table = """ CREATE TABLE IF NOT EXISTS calls (
                                    id integer PRIMARY_KEY,
                                    isEmergency integer,
                                    sessions_id integer,
                                    start_time timestamp,
                                    end_time timestamp,
                                    FOREIGN KEY (sessions_id)
                                        REFERENCES sessions(id)
                                );"""

    sql_create_sessions_table = """ CREATE TABLE IF NOT EXISTS sessions (
                                    id integer PRIMARY_KEY,
                                    calls_id integer NOT NULL,
                                    calls_count integer,
                                    start_time timestamp,
                                    end_time timestamp,
                                    isEmergency integer
                                );"""

    # channels table to be used later for region-matching
    sql_create_channels_table = """ CREATE TABLE IF NOT EXISTS channels (
                                    channel_num integer PRIMARY_KEY,
                                    sessions_id integer,
                                    FOREIGN KEY (sessions_id)
                                        REFERENCES sessions(id)
                                );"""

    conn = create_connection(database)
    if conn is not None:
        create_table(conn, sql_create_radios_table)
        create_table(conn, sql_create_calls_table)
        create_table(conn, sql_create_sessions_table)
        create_table(conn, sql_create_channels_table)
        conn.close()
    else:
        print("Error, cannot create the database connection")


def addCallToDB(id, isEmergency, start_time, end_time, radio, session):
    conn = create_connection(database)
    c = conn.cursor()
    sql = ''' INSERT INTO calls(id,
                                isEmergency,
                                start_time,
                                end_time,
                                radio_id,
                                session_id)
            VALUES(?,?,?,?,?,?) '''
    params = (id, isEmergency, start_time, end_time, radio, session)
    c.execute(sql, params)
    conn.commit()

    return c.lastrowid
