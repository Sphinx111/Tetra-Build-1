import igraph as ig


def main():
    global g
    g = ig.Graph()


def addCall(id, radio, sessionID):
    g.add_vertex(str(id), {"radio": radio, "session": str(sessionID)})
    g.add_edge(str(id), str(sessionID))


def addSession(id):
    g.add_vertex(str(id))


def removeObject(id):
    g.delete_vertices(str(id))
