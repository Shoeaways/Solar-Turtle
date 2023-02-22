from motion_planning import get_path_from_A_star

if __name__ == '__main__':
    start = (0, 0) # this is a tuple data structure in Python initialized with 2 integers
    goal = (9,0)
    obstacles = [(2,0),(2,-1),(5,0),(5,1),(8,2),(8,3)]
    path = get_path_from_A_star(start, goal, obstacles)
    print(path)
