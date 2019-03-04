import sys
import os
import requests as reqs
import math


class Mpwget:

    resources = []
    servers = []

    def parser(self, argv):
        """
        Split arguments depending of type.
        Servers start with :, otherwise will be resources.
        For example: obj1 obj2 :server1 :server2 obj3
        d will have: {"servers": ["server1", "server2"], "resources":["obj1","obj2","obj3"]}
        """
        for a in argv:
            if a[0] == ':':
                self.servers.append("http://%s" % a[1:] if a[0:5] != "http" else a[1:])
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

    def get_requests(self):
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
            partition_size = math.ceil(resource["size"] / len(self.servers))

            self.resources[i] = {
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
            print(resource)
            print("Fetching '%s'. Total size: %d bytes." % (resource["resource_name"], resource["size"]))
            content = b""
            for j, req in enumerate(sorted(resource["requests"], key=lambda x: x["offset"])):
                print("\tPacket %d -> Bytes: %d-%d. Server: %s"
                      % (j+1, req["offset"], req["offset"] + req["size"], req["server"]))
                headers = {"Range": "bytes=%d-%d" % (req["offset"], req["offset"] + req["partition_size"])}
                content += reqs.get("%s/%s" % (req["server"], resource["resource_name"]), headers=headers).content
            print("\tDone!\n\n%s\n" % ('-' * 80))
            self.resources[i]["content"] = content

    def save_resources(self):
        # Write results in folder
        for o in self.resources:
            with open('./resources/' + o["resource_name"].split('/')[-1], 'wb') as fd:
                fd.write(o["content"])

    def __init__(self, argv):
        # Remove older files or create folder
        if os.path.exists('./resources'):
            os.system("rm ./resources/* &> /dev/null")
        else:
            os.system("mkdir resources &> /dev/null")

        self.parser(argv)
        self.check_servers()
        self.get_requests()
        self.make_requests()

        self.save_resources()


if __name__ == "__main__":
    arguments = sys.argv[1:]
    mpwget = Mpwget(arguments)




