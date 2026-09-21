class Group:
    def __init__(self):
        self.objects = []

    def add(self, obj):
        self.objects.append(obj)

    def remove(self, obj):
        self.objects.remove(obj)

    def move(self, dx : int, dy : int):
        for obj in self.objects:
            obj.move(dx, dy)