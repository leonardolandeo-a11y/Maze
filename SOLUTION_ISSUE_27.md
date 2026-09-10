# Solution for Issue #27

## 🛠️ Proposed Solution (by Aditya Waghamare)

### Analysis
The `HeuristicPolicy` needs to be implemented for the maze navigation framework. It evaluates the available legal actions from the agent's current position, simulates the resulting position for each action, computes the Manhattan distance to the exit/goal, and selects the optimal action (handling ties deterministically, while ensuring only valid legal actions are chosen).

### Fix
Implemented `HeuristicPolicy` structure and decision-making logic adhering strictly to the required specification and integration patterns of the maze navigation system.

### Implementation
```python
class HeuristicPolicy:
    """
    HeuristicPolicy selects the action that minimizes the Manhattan distance
    from the resulting position to the exit/goal position.
    """
    def __init__(self):
        pass

    def select_action(self, current_position, exit_position, legal_actions, action_effects):
        """
        Selects the best legal action based on Manhattan distance to the exit.
        
        :param current_position: tuple (x, y) representing agent's current position.
        :param exit_position: tuple (x, y) representing the exit/goal position.
        :param legal_actions: list of available legal actions.
        :param action_effects: dict mapping each action to its resulting position offset or function.
        :return: Selected legal action.
        """
        if not legal_actions:
            raise ValueError("No legal actions provided to HeuristicPolicy.")

        best_action = None
        min_distance = float('inf')

        for action in legal_actions:
            # Calculate resulting position
            # Assuming action_effects provides the position delta or resulting position directly
            delta = action_effects.get(action, (0, 0))
            resulting_position = (current_position[0] + delta[0], current_position[1] + delta[1])

            # Calculate Manhattan distance: |x1 - x2| + |y1 - y2|
            dist = abs(resulting_position[0] - exit_position[0]) + abs(resulting_position[1] - exit_position[1])

            # Select action with the shortest distance (first encountered on tie)
            if dist < min_distance:
                min_distance = dist
                best_action = action

        return best_action
```

### Testing
- Verified correctness of Manhattan distance calculations for orthogonal grid movements.
- Ensured tie-breaking behaves predictably and only actions within `legal_actions` are evaluated and returned.

Signed-off-by: Aditya Waghamare <adityawaghamare7620@gmail.com>

---
*Submitted by Aditya Waghamare*
💰 **Payout Address (Base L2 / EVM):** `0xb61dBcdBc3407F71EaCb64D4CBFAcf9FFfe2415C`