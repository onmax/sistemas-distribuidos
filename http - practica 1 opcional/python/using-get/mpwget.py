import sys
import os
import requests as reqs
import math


class Mpwget:

    resources = []
    servers = []
    options = {
        "print": False,
        "save_requests": False
    }

    def parser(self, argv):
        """
        Split arguments depending of type.
        Servers start with :, otherwise will be resources.
        For example: obj1 obj2 :server1 :server2 obj3
        d will have: {"servers": ["server1", "server2"], "resources":["obj1","obj2","obj3"]}
        """
        for a in argv:
            if a[0] == '-':
                self.options["print"] = self.options["print"] or a == '--print'
                self.options["save_requests"] = self.options["save_requests"] or a == '--requests'
            elif a[0] == ':':
                self.servers.append("http://%s" % a[1:] if a[1:5] != "http" else a[1:])
            else:
                self.resources.append({"name": a})

    def check_servers(self):
        """
        Check if all servers are up and running. If one server is down, then it will be deleted from our list of servers
        :param: servers: contains all servers given by the user
        :return: servers: contains all servers given by the user and that are running
        """
        for s in self.servers:
            try:
                res = reqs.head(s)
                if res.status_code != 200:
                    self.servers.remove(s)
            except:
                self.servers.remove(s)

        if len(self.servers) == 0:
            print("No hay servidores disponibles")
            exit(1)

    def prepare_requests(self):
        """
            [
                {
                    "resouce_name": Name of the object,
                    "total_size": Size of the object in bytes,
                    "requests": [
                        {
                            "server": Server where the object is,
                            "size": Size of the partial part that needs to be fetched,
                            "offset": Where to start
                        },
                        ...
                    ]
                },
                ...
            ]
        """

        for i, resource in enumerate(self.resources):
            headers = reqs.head("%s/%s" % (self.servers[(i + 1) % len(self.servers)], resource["name"])).headers
            size = int(headers["content-length"])
            partition_size = math.ceil(size / len(self.servers))

            self.resources[i] = {
                "name": self.resources[i]["name"],
                "size": size,
                "type": headers["content-type"],
                "requests": [
                    {
                        "server": s,
                        "partition_size": partition_size - 1,
                        "offset": partition_size * i
                    } for i, s in enumerate(self.servers)
                ]
            }

    def make_requests(self):
        """
        Makes the requests, concatenate object and print status
        """
        for i, resource in enumerate(self.resources):
            self.resources[i]["content"] = b""
            if self.options["print"]:
                print("\nFetching '%s'. Total size: %d bytes." % (resource["name"], resource["size"]))
            for j, req in enumerate(sorted(resource["requests"], key=lambda x: x["offset"])):
                if self.options["print"]:
                    print("\tPacket %d -> Bytes: %d-%d. Server: %s"
                          % (j+1, req["offset"], req["offset"] + req["partition_size"], req["server"]))
                headers = {"Range": "bytes=%d-%d" % (req["offset"], req["offset"] + req["partition_size"])}
                self.resources[i]["content"] += reqs.get("%s/%s" % (req["server"], resource["name"]), headers=headers).content
            if self.options["print"]:
                print("\tDone!\n\n%s\n" % ('-' * 80))

    def save_resources(self):
        # Write results in folder resources
        for o in self.resources:
            try:
                with open('./resources/' + o["name"].split('/')[-1], 'wb') as fd:
                    fd.write(o["content"])
            except:
                print("%s: Hubo un error al intentar guardar el recurso %s.\n" % "mpwget", o["name"])

    def save_requests(self):
        try:
            with open('./requests_made', 'w') as fd:
                fd.write(self.resources)
        except:
            print("%s: Hubo un error al intentar guardar la información de las peticiones.\n" % "mpwget")

    def __init__(self, argv):
        # Remove older files or create folder
        if os.path.exists('./resources'):
            os.system("rm ./resources/* &> /dev/null")
        else:
            os.system("mkdir resources &> /dev/null")

        self.parser(argv)
        self.check_servers()
        self.prepare_requests()
        self.make_requests()

        self.save_resources()

        if self.options["save_requests"]:
            self.save_requests()



if __name__ == "__main__":
    arguments = sys.argv[1:]
    mpwget = Mpwget(arguments)




