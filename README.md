# Controls.
**Left**/**Right** arrows: Switch AI modes\
**Spacebar**: Randomize Path & Obstacles\
**Enter**: Toggle Visualizations
# Remarks.
**Seek**, **Flee**, **Pursue**, **Evade**, **Arrive**, **Wander** and **Path Following** works well.\
**Separation and Collision Avoidance** (multiple agents) works fine, but if two agents are heading straight toward each other they will pass through each other.\
**Obstacle and Wall Avoidance** works well with isolated objects, but for multiple intersecting objects, the agent migth pass through the object.\
The **Combined Steering Behaviors** combines (with multiple agents) **Path Following**, **Separation** and **Obstacle and Wall Avoidance**.\
The agents try to follow the path, but when other agents are nearby the Separation will be added to the Path behavior. When a wall is seen from the raycast, the Obstacle and Wall Avoidance behavior will override the other behaviors. The issues from **Separation** and **Obstacle and Wall Avoidance** are more noticeable in this mode.
