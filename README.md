# Field Fest: a Rhythm Game with a GPU-Instanced Concert Crowd

Computer Graphics Fundamentals, Final Project (Astana IT University, 2026–2027)

> **Team:** Altynay Yertay, Nuray Nuraly, Zaure Tolegen · **Group:** SE-2416 · **Topic #:** N
> *(Edit these here and in `src/Config.h` so they match the topic sheet.)*

A real-time OpenGL 3.3 core rhythm game set at an open-air night concert in one of three
locations (meadow, beach with the sea, winter with northern lights). The sun sets, a crowd of
**3 000 – 50 000 people** jumps to the music, artists perform on stage, and moving spotlights,
lasers, pyro and fireworks fire when **you** hit the notes. The headline technique is the
**GPU-instanced crowd**: up to 50 000 people in one `glDrawArraysInstanced` call; key **I** switches to
one draw call per person for comparison. The music is synthesized in code (no audio files) and drives
the lights, the crowd and the LED screen.

---

## 1. Requirements

| | Windows 10/11 | macOS (Apple Silicon / Intel) | Linux |
|---|---|---|---|
| Compiler | Visual Studio 2019 or 2022 (MSVC, "Desktop development with C++") | Apple Clang (Xcode Command Line Tools) | GCC 9+ / Clang 10+ |
| Build tool | CMake ≥ 3.16 (bundled with Visual Studio) | CMake ≥ 3.16 (`brew install cmake`) | CMake ≥ 3.16 |
| GPU | OpenGL 3.3 core | OpenGL 3.3+ core (macOS provides 4.1) | OpenGL 3.3 core (+ X11/Wayland dev packages for GLFW) |

All libraries are bundled in `third_party/`, so the build does not download anything.
Sound plays through the default output device; without one the show runs silently.

## 2. Build

### Windows (Visual Studio), from a "Developer PowerShell" or `cmd`
```bat
cmake -S . -B build
cmake --build build --config Release
build\Release\FieldFest.exe
```
You can also open `build\FieldFest.sln` in Visual Studio, select **Release**, and press **F5**.
`FieldFest` is already set as the startup project.

### macOS / Linux
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/FieldFest
```

Shaders are loaded from files at runtime. The program first looks in the source folder
(`shaders/`, which is what makes **F5 live reload** work) and then in `shaders/` next to the
executable, which the build copies there automatically.

## 3. Controls

| Key | Action |
|---|---|
| **← ↓ ↑ →** (or **D F J K**) | hit the notes: left spotlights / lasers / pyro / right spotlights |
| **← / →** on the start screen | choose the song |
| **↑ / ↓** on the start screen | choose the location |
| **Enter** | start / restart the show |
| **R** | reset |
| **Mouse drag** / **wheel** | rotate / zoom the camera |
| **Tab** | next camera view: behind the crowd, from the stage, aerial, front row, artists close-up |
| **C** | free-fly camera (**WASD**, **Space/Ctrl** up/down, **Shift** fast) |
| **O** | autopilot: hits every note perfectly (demo / video recording) |
| **M** | mute music |
| **I** | **toggle crowd instancing (headline technique)**: 1 draw call vs one draw call per person |
| **T** | toggle wind animation (blades go to their rest pose) |
| **P** | pause (music too) |
| **Q / E** | rotate the sun (azimuth) |
| **Z / X** | lower / raise the sun |
| **1 2 3 4** | crowd size 1 000 / 3 000 / 20 000 / 50 000 |
| **V** | VSync on/off (turn it off when measuring FPS) |
| **F5** | reload all shaders from disk (edit live during the defence) |
| **F12** | save a screenshot to `screenshots/` (BMP) |
| **F1 / H** | help overlay |
| **Esc** | quit |

**Game:** you are the light director. Choose a song with **← / →** on the start screen and press
**Enter**: the show starts at golden hour and night falls during the song.

| Song | Style | Tempo | Length | Notes | Difficulty |
|---|---|---|---|---|---|
| Sunset Drive | house, Am–F–C–G | 128 BPM | 1:30 | 184 | normal |
| Lazy Fields | lo-fi chill, swung, Fmaj7–Em7–Dm7–Cmaj7 | 96 BPM | 2:00 | 123 | easy |
| Neon Rush | hard dance, rolling bass, Dm–B♭–F–C | 150 BPM | 1:16 | 210 | hard |

Each song has its own sound, light colours and best score (`best_score*.txt`).

| Location | What is special |
|---|---|
| Meadow | tall grass with wildflowers, leafy trees, fireflies at night |
| Beach | sand dunes with sparse grass, palms, the sea behind and beside the stage (waves, Fresnel reflections, foam) |
| Winter | snow with sparkling ice crystals, frosted grass, snowy pines, falling snow, northern lights |

* Notes fall down 4 lanes. Press the matching arrow when a note reaches the line:
  **PERFECT** (±55 ms) = 300 points, **GOOD** (±120 ms) = 100 points, times the combo multiplier (up to ×4).
* Every hit fires its effect: white flash of the left/right spotlights, lasers + strobe, or fire columns.
* Good timing raises the crowd's **hype**: they jump higher, raise their arms, the lights get brighter.
  Misses (and random key mashing) lower it. At zero hype the crowd goes home: you lose.
* Finish the song: **ENCORE!** with a fireworks finale, accuracy, grade (S/A/B/C/D) and max combo.
  Every 25 combo launches a firework.
* The song has sections (Intro, Build-up, **Drop**, Breakdown, Build-up 2, **Final drop**); in the
  breakdown the spotlights point at the sky and people hold up their phone lights.

**Your own songs:** put `.mp3`, `.wav` or `.flac` files into `songs/`. They appear in the menu
as "your track". The game decodes the file (miniaudio), finds the tempo and the beats by itself
and builds the notes and the light show from the sound (see section 5). A DJ is on stage for
every song; for your own songs two rappers or a singer join (chosen from the file name in
`App::selectSong`). Do not put copyrighted songs into the submitted ZIP or the YouTube video.

**Best-looking shots:** the drop (from 0:30) and the final drop (from 1:15) at night, camera
"behind the crowd" or "front row". Press **O** for autopilot to record without playing.

### Command line (useful for report screenshots)
```
FieldFest --shot out.bmp --frames 150 [--song N] [--location 0-2] [--elapsed 33] [--autopilot] [--view 0-4] [--crowd N] [--naive] [--mute] [--nohud] [--size 1280 720]
FieldFest --bench
```
`--shot` renders N frames, saves one screenshot, and exits (no sound). `--elapsed 33` starts the
show 33 s into the song (the first drop). `--view` picks the camera, `--nohud` hides the panels. `--naive` starts with
crowd instancing OFF. `--bench` runs the performance measurement (section 6).

## 4. Project structure / modules

| Module | Files | Owner |
|---|---|---|
| Renderer core: window, frame loop, render passes, GPU timer | `src/App.*`, `src/FrameUniforms.*`, `src/GpuTimer.*`, `src/Screenshot.*`, `src/Shader.*`, `src/MathUtil.h` | *name* |
| **Headline technique: instanced crowd (+ naive path, benchmark)** | `src/Crowd.*`, `shaders/crowd.*`, `src/App.cpp` (`benchTick`) | *name* |
| Stage, artists, trees, light beams | `src/Stage.*`, `src/Performers.*`, `src/Props.*`, `src/Beams.*`, `src/Geometry.h`, `shaders/props.*`, `shaders/mesh.*`, `shaders/beam.*` | *name* |
| Environment: locations, terrain, sea, sky, stars, aurora | `src/Location.*`, `src/GridMesh.*`, `src/Terrain.*`, `src/Water.h`, `src/Sky.*`, `src/Grass.*`, `shaders/common.glsl`, `shaders/ground.*`, `shaders/grass.*`, `shaders/water.*`, `shaders/sky.*` | *name* |
| Music, rhythm game, light show, HUD | `src/Music.*`, `src/Track.*`, `src/Audio.*`, `src/Game.*`, `src/Show.*`, `src/GlowSprites.*`, `src/Hud.*`, `src/Camera.*`, `shaders/glow.*` | *name* |
| Report, video, presentation | `report/` | *name* |

## 5. How it works (short)

* **Crowd instancing (headline technique):** one 9-box person mesh (324 vertices), up to 50 000
  instances (position, facing, size, shirt hue, phase), sorted from the stage backwards so the
  first N fill the field from the front. Instanced: one `glDrawArraysInstanced`; naive (key **I**):
  one `glDrawArrays` per person with the data set by `glVertexAttrib4fv`. `crowd.vert` rotates the arms around the shoulder (`rotZ`), sways the
  body and makes everyone jump on the beat with a parabola `4f(1 − f)`.
* **Artists on stage (hierarchical modelling, `src/Performers.*`):** a DJ behind the booth and,
  for your own songs, two rappers with microphones. Each is a tree of box parts:
  body → head, legs, upper arms → forearms → hands, fingers, microphone. Every frame
  `M_child = M_parent · T(pivot) · R · T(−pivot)`; the angles come from the beat (bounce, head
  nod, walking with leg swing, peace sign pumping), and the vertices are transformed on the CPU.
  Two warm follow-spots (point lights 8 and 9) light them.
* **Lighting (`common.glsl`):** Lambert sun + ambient, back-lit translucency of the blades,
  8 moving **spotlights** (cone test `smoothstep(cosθ, …, dot(−L, dir))`, falloff `1/(1 + k·d²)`),
  10 point lights (LED screen, pyro, fireworks, follow-spots), a soft limit `L/(1 + 0.15L)`, and distance fog.
* **Visible beams:** an open cone mesh drawn instanced with additive blending; brightness
  `|n·v|² · (1 − t)^1.6` with drifting haze. Lasers are the same cones, very thin.
* **Particles:** sparks, fire columns and fireworks are CPU-simulated (gravity + drag) and drawn
  as instanced camera-facing glow sprites.
* **Music:** three songs share one 48-bar structure; a `Song` struct sets the tempo, chords,
  swing, kick pitch, bass style (off-beat / rolling 16ths / long notes), melody sound, chart
  difficulty and a hue shift of the light colours. `music::sample(t)` is a pure function of time: kick (pitch sweep
  `φ = 2π(48t + 112(1 − e^(−28t))/28)`), clap and hats from hashed noise, a plucky bass, a 16th-note
  arpeggio and a pad over Am–F–C–G, with sidechain "pumping". miniaudio calls it on the audio
  thread; the game clock follows the audio clock. The same pattern gives the light levels and the
  note chart, so everything is exactly on the beat.
* **Your own songs (`src/Track.*`):** the file is decoded to 48 kHz stereo, then every 10.7 ms
  a 1024-point FFT (own radix-2 implementation) gives the **spectral flux**: the sum of positive
  jumps of the log-magnitude spectrum, i.e. "something just started". Tempo: for every BPM from
  60 to 200 in 0.01 steps, the strongest onsets are folded into one beat (`time mod beat`); the
  tempo where they pile up at one phase most sharply wins, and that phase gives the beat grid.
  Notes are the strongest onsets snapped to the grid (beats preferred over half-beats); bass-heavy
  onsets go to the laser lane, the very strongest to pyro. Loud / quiet parts become drop /
  breakdown sections for the lights.
* **Locations (`src/Location.*`):** one table of parameters (ground and grass colours, grass height
  and coverage, sky tints) plus a location-dependent `terrainHeight()` in C++ and GLSL. The beach
  terrain slopes into the **sea**: a camera-following grid displaced by three travelling sine
  waves with analytic normals; `water.frag` mixes a turquoise deep colour with the reflected sky by
  the Schlick **Fresnel** term `0.02 + 0.98(1 − n·v)^5`, adds sun glitter, reflections of every
  concert light and foam where the water is shallow. Winter adds ice-crystal glints on the snow,
  4 500 falling snowflakes (positions are a pure function of time) and an **aurora** in the sky
  shader (wavy bands `exp(−x²)` with vertical rays). Trees (leafy, palm, snowy pine) are box meshes,
  one instanced draw per kind, swaying in the wind.

## 6. Measuring performance

Run `./build/FieldFest --bench`. It freezes one moment of the first drop at night (so only the
crowd changes between the runs), turns VSync off and measures 5 crowd sizes (1 000, 3 000, 10 000, 20 000, 50 000) with crowd instancing
ON and OFF: 1.5 s warm-up + 3 s of averaging each. The table is printed and saved as
`benchmark.csv` (a copy from an Apple M4 at 2560×1440 is in `report/benchmark.csv`).
Manually: **1–4** change the crowd size, **I** toggles instancing, the HUD shows FPS, crowd draw
calls and crowd GPU time (`GL_TIME_ELAPSED` query).

## 7. Third-party code and licenses

| Library | Use | License | Source |
|---|---|---|---|
| GLFW 3.4 | window, input, GL context | zlib/libpng | https://github.com/glfw/glfw |
| GLAD 2 (generated, GL 3.3 core) | OpenGL function loader | generated code: public domain / CC0; GL registry: Apache-2.0 | https://github.com/Dav1dde/glad |
| GLM 1.0.1 | vector/matrix math | MIT (Happy Bunny) | https://github.com/g-truc/glm |
| Dear ImGui 1.91.9b | HUD text, panels, note highway | MIT | https://github.com/ocornut/imgui |
| miniaudio 0.11.21 | audio output, MP3 / WAV / FLAC decoding | public domain or MIT-0 | https://github.com/mackron/miniaudio |

No external models, textures or sounds are used. The ground texture, the sky, all geometry and
the music are generated procedurally in code.

## 8. Screenshots

All screenshots are rendered by the program itself with `--shot` (1280×720 window, 2× Retina =
2560×1440, saved as 1920×1080 JPEG). Unless noted, the show is at the first drop at night (`--elapsed 36`)
with the autopilot on and the HUD hidden (`--nohud`).

### 8.1 The show from start to finish (Meadow, *Sunset Drive*)

| | |
|---|---|
| <img src="report/screenshots/a1_menu.jpg" alt="Start screen: song and location menu, golden hour" width="420"> | <img src="report/screenshots/a2_golden_hour_start.jpg" alt="0:04 - the show starts at golden hour" width="420"> |
| Start screen: song and location menu, golden hour | 0:04 - the show starts at golden hour |
| <img src="report/screenshots/a3_sunset_buildup.jpg" alt="0:22 - build-up, the sun goes down" width="420"> | <img src="report/screenshots/b_meadow_1_behind_crowd.jpg" alt="0:36 - the drop at night: spotlights, lasers, pyro" width="420"> |
| 0:22 - build-up, the sun goes down | 0:36 - the drop at night: spotlights, lasers, pyro |
| <img src="report/screenshots/a4_breakdown_phone_lights.jpg" alt="0:52 - breakdown: beams point at the sky, phone lights in the crowd" width="420"> | <img src="report/screenshots/a5_final_drop.jpg" alt="1:20 - final drop: rainbow spotlights" width="420"> |
| 0:52 - breakdown: beams point at the sky, phone lights in the crowd | 1:20 - final drop: rainbow spotlights |
| <img src="report/screenshots/a6_gameplay_hud.jpg" alt="Gameplay with the HUD and the note highway" width="420"> | <img src="report/screenshots/a7_encore.jpg" alt="Win: ENCORE! with the score, accuracy and grade" width="420"> |
| Gameplay with the HUD and the note highway | Win: ENCORE! with the score, accuracy and grade |
| <img src="report/screenshots/a8_crowd_went_home.jpg" alt="Lose: the hype dropped to zero, the crowd went home" width="420"> |  |
| Lose: the hype dropped to zero, the crowd went home |  |

### 8.2 All camera views in all locations (key **Tab**, `--view 0-4`, `--location 0-2`)

| View | Meadow | Beach | Winter |
|---|---|---|---|
| **Behind the crowd** | <img src="report/screenshots/b_meadow_1_behind_crowd.jpg" alt="meadow - Behind the crowd" width="280"> | <img src="report/screenshots/b_beach_1_behind_crowd.jpg" alt="beach - Behind the crowd" width="280"> | <img src="report/screenshots/b_winter_1_behind_crowd.jpg" alt="winter - Behind the crowd" width="280"> |
| **From the stage** | <img src="report/screenshots/b_meadow_2_from_stage.jpg" alt="meadow - From the stage" width="280"> | <img src="report/screenshots/b_beach_2_from_stage.jpg" alt="beach - From the stage" width="280"> | <img src="report/screenshots/b_winter_2_from_stage.jpg" alt="winter - From the stage" width="280"> |
| **Aerial** | <img src="report/screenshots/b_meadow_3_aerial.jpg" alt="meadow - Aerial" width="280"> | <img src="report/screenshots/b_beach_3_aerial.jpg" alt="beach - Aerial" width="280"> | <img src="report/screenshots/b_winter_3_aerial.jpg" alt="winter - Aerial" width="280"> |
| **Front row** | <img src="report/screenshots/b_meadow_4_front_row.jpg" alt="meadow - Front row" width="280"> | <img src="report/screenshots/b_beach_4_front_row.jpg" alt="beach - Front row" width="280"> | <img src="report/screenshots/b_winter_4_front_row.jpg" alt="winter - Front row" width="280"> |
| **Artists close-up** | <img src="report/screenshots/b_meadow_5_artists.jpg" alt="meadow - Artists close-up" width="280"> | <img src="report/screenshots/b_beach_5_artists.jpg" alt="beach - Artists close-up" width="280"> | <img src="report/screenshots/b_winter_5_artists.jpg" alt="winter - Artists close-up" width="280"> |

### 8.3 Every song has its own light colours

| *Sunset Drive* (house, 128 BPM) | *Lazy Fields* (lo-fi, 96 BPM) | *Neon Rush* (hard dance, 150 BPM) |
|---|---|---|
| <img src="report/screenshots/b_meadow_1_behind_crowd.jpg" alt="Sunset Drive" width="280"> | <img src="report/screenshots/c1_song_lazy_fields.jpg" alt="Lazy Fields" width="280"> | <img src="report/screenshots/c2_song_neon_rush.jpg" alt="Neon Rush" width="280"> |

### 8.4 Headline technique: instanced crowd (key **I**, keys **1-4**)

The same moment with 50 000 people: with instancing the whole crowd is **1** draw call, without it
**50 000** draw calls (see the FPS, draw calls and crowd GPU time in the top-right panel; measured
numbers are in section 6 and `report/benchmark.csv`).

| Instancing ON, 50 000 people | Instancing OFF, 50 000 people |
|---|---|
| <img src="report/screenshots/d1_instancing_on_50k.jpg" alt="Instancing ON" width="420"> | <img src="report/screenshots/d2_instancing_off_50k.jpg" alt="Instancing OFF" width="420"> |

| 1 000 people | 20 000 people |
|---|---|
| <img src="report/screenshots/d3_crowd_1000.jpg" alt="1000 people" width="420"> | <img src="report/screenshots/d4_crowd_20000.jpg" alt="20000 people" width="420"> |

### 8.5 Artists on stage (hierarchical models)

The DJ is always there (see *Artists close-up* above); for your own songs two rappers or a singer join.

| Two rappers + DJ | Singer + DJ |
|---|---|
| <img src="report/screenshots/e1_rappers_duo.jpg" alt="Two rappers and the DJ" width="420"> | <img src="report/screenshots/e2_singer.jpg" alt="Singer and the DJ" width="420"> |
