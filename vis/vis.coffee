if (!Detector.webgl) then Detector.addGetWebGLMessage

scene = new (THREE.Scene)
camera_far = 5000
camera = new (THREE.PerspectiveCamera)(75, window.innerWidth / window.innerHeight, 0.1, camera_far)
renderer = new (THREE.WebGLRenderer)((antialias: true))
renderer.setSize window.innerWidth, window.innerHeight
document.body.appendChild renderer.domElement

window.addEventListener('resize', () ->
  camera.aspect = window.innerWidth / window.innerHeight
  camera.updateProjectionMatrix()
  renderer.setSize window.innerWidth, window.innerHeight)

### utility functions ###
vec3 = (x, y, z) -> new (THREE.Vector3)(x, y, z)
vec3_from_array = (p) ->
    return vec3(p[0], p[1], if p.length > 2 then p[2] else 1)
PI = Math.PI

add_to = (c, o) ->
  c.push(o)
  return o

make_point = (collection, size, color, opacity=1.0, detail=15) ->
 return add_to(collection, new (THREE.Mesh)(
  new (THREE.SphereGeometry)(size, detail, detail),
  new (THREE.MeshLambertMaterial)(
    color: color,
    transparent: opacity != 1.0, opacity: opacity)))

make_plane = (collection, size, color, opacity=1.0) ->
 return add_to(collection, new (THREE.Mesh)(
  new (THREE.PlaneGeometry)(size, size)
  new (THREE.MeshBasicMaterial)((
    color: color, side: THREE.DoubleSide,
    transparent: opacity != 1.0, opacity: opacity))))

make_line = (collection, here, there, color, width=1, extend=true) ->
  # get vectors and extend the line's length to 2 * camera_far
  v1 = if here instanceof THREE.Vector3 then here else vec3_from_array(here)
  v2 = if there instanceof THREE.Vector3 then there else vec3_from_array(there)
  if there
    v1_orig = v1.clone()
    v1.sub(v2)
    v1.setLength(camera_far)
    v1.add(v2)
    v2.sub(v1_orig)
    v2.setLength(camera_far)
    v2.add(v1_orig)

  geometry = new (THREE.Geometry)
  geometry.vertices.push(v1, v2)
  return add_to(collection, new (THREE.Line)(geometry,
    new (THREE.LineBasicMaterial)((color: color, linewidth: width))))

make_p_point = (x, y, color) ->
  o1 = make_point(r2_objects, 0.01, color)
  o1.position.z = 1
  o1.position.x = x
  o1.position.y = y
  o2 = make_line(root_objects, [-x, -y, -1], [x, y, 1], color)

  o3 = make_point(s2_objects, 0.0075, color)
  o3.position.copy(vec3(x, y, 1).normalize())
  o4 = make_point(s2_objects, 0.0075, color)
  o4.position.copy(vec3(-x, -y, -1).normalize())

  return [o1, o2, o3, o4]

make_p_line = (p1, p2, color, opacity=1.0, draw_points=false) ->
  o = make_plane(root_objects, 3, color, opacity)
  v1 = vec3(p1[0], p1[1], 1)
  v2 = vec3(p2[0], p2[1], 1)
  make_line(r2_objects, v1.clone(), v2.clone(), color, 9)
  if draw_points
    make_p_point(p1[0], p1[1], color)
    make_p_point(p2[0], p2[1], color)
  v1.cross(v2)
  angle = v1.angleTo(vec3(0,0,1))
  o.rotation.setFromQuaternion(new (THREE.Quaternion)().setFromAxisAngle(vec3(1, 0, 0), angle))
  return o


### setup scene ###
s2_objects = []
r2_objects = []
root_objects = []

make_point(root_objects, 0.02, 0xffffff)
make_plane(root_objects, 2, 0xbbffbb)

make_p_point(0, 0, 0xff0000)
make_p_point(0.5, 0.5, 0xff0000)
make_p_line([0.25, 0.25], [-0.25, 0.25], 0xff00ff, 0.5, true)

s2 = make_point(s2_objects, 1, 0xffffff, 0.2, 90)
s2_objects.push s2

r2 = make_plane(r2_objects, 2, 0xffffbb)
r2.position.z = 1

transform = (new (THREE.Matrix4)).set(2, 0, 0, 0,
                                      0, 1, 0, 0,
                                      0, 0, 1, 0,
                                      0, 0, 0, 0)
scene.add obj for obj in root_objects
scene.add obj for obj in s2_objects
scene.add obj for obj in r2_objects
scene.add new (THREE.AmbientLight)(0x404040)
scene.add new (THREE.HemisphereLight)(0xffffff, 0x404040, 1)

### place camera and render ###

controls = new (THREE.OrbitControls)(camera, renderer.domElement)

camera.position.z = 2
time = -PI * 3/4
camera.position.y = 3 * Math.sin time
camera.position.x = 3 * Math.cos time
camera.lookAt(vec3(0, 0, 0))
controls_active = true

render = ->
  requestAnimationFrame render
  renderer.render scene, camera
  if controls_active
    controls.update
  else
    time += 0.005
    camera.position.y = 3 * Math.sin time
    camera.position.x = 3 * Math.cos time
    camera.lookAt(vec3(0, 0, 0))
  return

render()
