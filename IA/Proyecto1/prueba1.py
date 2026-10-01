import heapq

class SokobanProblem:
    """
    Clase que formaliza el problema de Sokoban de una sola caja.
    """
    def __init__(self, grid):
        """
        Inicializa el tablero a partir de una matriz de texto.
         Leyenda: 'W' = Pared, 'R' = Robot, 'B' = Caja, 'T' = Meta, ' ' = Vacío
        """
        self.walls = set()
        self.target = None
        start_robot = None
        start_box = None

        for r, row in enumerate(grid):
            for c, val in enumerate(row):
                if val == 'W':
                    self.walls.add((r, c))
                elif val == 'R':
                    start_robot = (r, c)
                elif val == 'B':
                    start_box = (r, c)
                elif val == 'T':
                    self.target = (r, c)

        # Estado mínimo: ((x_robot, y_robot), (x_caja, y_caja))
        self.initial_state = (start_robot, start_box)

    def heuristic(self, state):
        """
        Calcula la Distancia Manhattan de la caja a la meta.
        h(n) = |x_caja - x_meta| + |y_caja - y_meta|
        """
        _, (bx, by) = state
        tx, ty = self.target
        return abs(bx - tx) + abs(by - ty)

    def get_successors(self, state):
        """
        Genera los estados sucesores legales (acciones: Arriba, Abajo, Izquierda, Derecha).
        """
        (rx, ry), (bx, by) = state
        successors = []
        actions = [(-1, 0), (1, 0), (0, -1), (0, 1)] # Movimientos ortogonales

        for dx, dy in actions:
            nrx, nry = rx + dx, ry + dy

            # 1. Movimiento ilegal: El robot choca con pared
            if (nrx, nry) in self.walls:
                continue

            # 2. Intento de empuje de la caja
            if (nrx, nry) == (bx, by):
                nbx, nby = bx + dx, by + dy
                # Ilegal si la posición detrás de la caja es una pared
                if (nbx, nby) in self.walls:
                    continue
                new_state = ((nrx, nry), (nbx, nby))
            else:
                # Movimiento libre del robot
                new_state = ((nrx, nry), (bx, by))

            successors.append((new_state, 1)) # Costo uniforme = 1
            
        return successors

    def is_goal(self, state):
        """Evalúa si la caja alcanzó la coordenada meta."""
        return state[1] == self.target


def a_star_search(problem):
    """
    Ejecuta la Búsqueda A* utilizando Búsqueda en Grafo para evitar ciclos.
    """
    start = problem.initial_state
    # Cola de prioridad: almacena tuplas (f_score, g_score, estado_actual, camino)
    frontier = []
    heapq.heappush(frontier, (problem.heuristic(start), 0, start, [start]))
    
    # Tabla Hash de estados visitados guardando el costo mínimo g(n)
    visited = {start: 0}
    nodes_expanded = 0

    while frontier:
        f, g, current, path = heapq.heappop(frontier)

        if problem.is_goal(current):
            return path, g, nodes_expanded

        nodes_expanded += 1

        for next_state, cost in problem.get_successors(current):
            new_g = g + cost
            # Si el estado no ha sido visitado o se encontró un camino más corto
            if next_state not in visited or new_g < visited[next_state]:
                visited[next_state] = new_g
                h = problem.heuristic(next_state)
                f_score = new_g + h
                heapq.heappush(frontier, (f_score, new_g, next_state, path + [next_state]))

    return None, float('inf'), nodes_expanded


# Tablero de prueba 8x8
sample_grid = [
    "WWWWWWWW",
    "W R    W",
    "W  W B W",
    "W  W   W",
    "W   T  W",
    "WWWWWWWW"
]

prob = SokobanProblem(sample_grid)
path, cost, expanded = a_star_search(prob)

print(f"Solución encontrada en {cost} pasos.")
print(f"Nodos expandidos: {expanded}")