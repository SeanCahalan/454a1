// segment.h


class Segment {
 public:

  vec3 tail, head;

  Segment( vec3 t, vec3 h )
    { tail = t; head = h; }


  float max(float x, float y){
    return x > y ? x : y;
  }  

  float min(float x, float y){
    return x < y ? x : y;
  }  

  bool onSegment(vec3 p, vec3 q, vec3 r) 
  { 
      if (q.x <= max(p.x, r.x) && q.x >= min(p.x, r.x) && 
          q.y <= max(p.y, r.y) && q.y >= min(p.y, r.y)) 
        return true; 
      else
        return false; 
  } 
    
  int orientation(vec3 p, vec3 q, vec3 r) 
  { 
      float val = (q.y - p.y) * (r.x - q.x) - 
                (q.x - p.x) * (r.y - q.y); 
    
      if (val == 0) return 0;  // colinear 
    
      return (val > 0)? 1: 2; // CW or CCW 
  }

  bool intersects( Segment const& s2 ) {
    // YOUR CODE HERE

    vec3 p1 = tail;
    vec3 q1 = head;
    vec3 p2 = s2.tail;
    vec3 q2 = s2.head;

    int o1 = orientation(p1, q1, p2); 
    int o2 = orientation(p1, q1, q2); 
    int o3 = orientation(p2, q2, p1); 
    int o4 = orientation(p2, q2, q1); 
  
    // General case 
    if (o1 != o2 && o3 != o4) 
        return true; 

    if (o1 == 0 && onSegment(p1, p2, q1)) return true; 
  
    if (o2 == 0 && onSegment(p1, q2, q1)) return true; 
  
    if (o3 == 0 && onSegment(p2, p1, q2)) return true; 
  
    if (o4 == 0 && onSegment(p2, q1, q2)) return true; 
  
    return false;
    
  }
};


