class DiamondSquare:
    def __init__(self, size, roughness, high):
        self.size = (2 ** size) + 1
        self.max = self.size - 1
        self.roughness = roughness
        self.high=high
        self.make_grid(self.size,self.high)
        self.divide(self.max)
    def set(self, x, y, val):
        self.grid[x + self.size * y] = val;
    def get(self, x, y):
        if (x < 0 or x > self.max or y < 0 or y > self.max):
            return -1
        return self.grid[x + self.size * y]
    def divide(self, size):
        import random
        x = round(size / 2)
        y = round(size / 2)
        half = round(size / 2)
        scale = self.roughness * size
        if (half < 1):
            return
        for y in range(half, self.max, size):
            for x in range(half, self.max, size):
                s_scale = random.uniform(0, 1) * scale * 2 - scale
                self.square(x, y, half, s_scale)
        for y in range(0, self.max + 1, half):
            for x in range((y + half) % size, self.max + 1, size):
                d_scale = random.uniform(0, 1) * scale * 2 - scale
                self.diamond(x, y, half, d_scale)
        self.divide(round(size / 2)) 
    def square(self, x, y, size, scale):
        top_left = self.get(x - size, y - size)
        top_right = self.get(x + size, y - size)
        bottom_left = self.get(x + size, y + size)
        bottom_right = self.get(x - size, y + size)
        average = round(((top_left + top_right + bottom_left + bottom_right) / 4))
        self.set(x, y, average + scale)
    def diamond(self, x, y, size, scale):
        top = self.get(x, y - size)
        right = self.get(x + size, y)
        bottom = self.get(x, y + size)
        left = self.get(x - size, y)
        average = round(((top + right + bottom + left) / 4))
        self.set(x, y, average + scale)
    def make_grid(self, size,high):
        import random
        self.grid = []
        for x in range(size * size):
            self.grid.append(-1)
        self.set(0, 0, random.randint(0,self.high))
        self.set(self.max, 0, random.randint(0,self.high))
        self.set(self.max, self.max, random.randint(0,self.high))
        self.set(0, self.max, random.randint(0,self.high))
    def get_grid(self):
        return self.grid
def Matrix(s,r,h):
    #s - размер мира
    #r - коэффициент случайности
    #h - начальная высота
    a=DiamondSquare(s,r,h)
    m_old=a.get_grid()
    m=[]
    for i in range(a.size):
        m.append(m_old[i*a.size:i*a.size+a.size])
    maximum=m[0][0]
    minimum=m[0][0]
    for i in range(len(m)):
        for j in range(len(m[i])):
            m[i][j]=round(m[i][j],0)
            if m[i][j]>maximum:
                maximum=m[i][j]
            if m[i][j]<minimum:
                minimum=m[i][j]
    return tuple([m,maximum,len(m),minimum])
