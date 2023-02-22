def neighbors(current):
    # define the list of 4 neighbors
    neighbors = [(1,0),(-1,0),(0,1),(0,-1)]
    return [ (current[0]+nbr[0], current[1]+nbr[1]) for nbr in neighbors ]

def heuristic_distance(candidate, goal):
    distance = abs(candidate[0] - goal[0]) + abs(candidate[1] - goal[1])
    return distance

def get_path_from_A_star(start, goal, obstacles):
    # input  start: integer 2-tuple of the current grid, e.g., (0, 0)
    #        goal: integer 2-tuple  of the goal grid, e.g., (5, 1)
    #        obstacles: a list of grids marked as obstacles, e.g., [(2, -1), (2, 0), ...]
    # output path: a list of grids connecting start to goal, e.g., [(1, 0), (1, 1), ...]
    #   note that the path should contain the goal but not the start
    #   e.g., the path from (0, 0) to (2, 2) should be [(1, 0), (1, 1), (2, 1), (2, 2)] 
    
    while goal in obstacles:
        print('Goal is not reachable')
        break

    #OPEN List
    open_list = []
    open_list = [(0,start)]
    #CLOSED List
    closed_list = []
    #Parent list
    parent = {}
    parent[start] = None
    #Past_cost
    past_cost = {}
    past_cost[start] = 0
    
    while open_list:
        open_list.sort()
        current = open_list.pop(0)[1]
        closed_list.append(current)
        #check if current is goal
        if current == goal:
            #break or add path function
            path = []
            
            while current != start:
                
                path.append(current)
                current = parent[current]
            path.reverse()
            return path

        for candidate in neighbors(current):
            if candidate in obstacles:
                continue
            tentative_cost = past_cost[current]+1
            if candidate not in past_cost or tentative_cost < past_cost[candidate]:
                past_cost[candidate] = tentative_cost
                parent[candidate] = current
                heuristic = heuristic_distance(candidate, goal)
                new_cost = past_cost[candidate] + heuristic
                open_list.append((new_cost, candidate))
            #sort open_list
            

    #path construction
        #either use the closed_list
        #either use the parent
        #path doesn't include the start
    
    return path
