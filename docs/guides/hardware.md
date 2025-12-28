# Hardware & Mechanical Design (Meca)

## Overview

The Meca folder contains 3D models and mechanical designs for the robot's physical structure. All models are designed for 3D printing or CNC manufacturing.

**Location**: `Meca/3D/`

## 3D Model Components

### Head Assembly

**Location**: `Meca/3D/Head/`

Contains 3D models for the robot's head structure:
- Head case (main housing)
- Eye housing (for display or RoboEyes)
- Servo mounting brackets
- Cable management clips

### Microphone Assembly

**Location**: `Meca/3D/INMP441/`

Mounting bracket for INMP441 I2S microphone:
- Mounting plate for microphone module
- Dust protection cover
- Cable clips for I2S wires

### OLED Display Housing

**Location**: `Meca/3D/Oled_1.3/`

Enclosure for 1.3" OLED display:
- Display frame/bezel
- Mounting hardware slots
- Cable routing

### Speaker Housing

**Location**: `Meca/3D/Speaker/`

Enclosure and mounting for speaker:
- Speaker mount
- Enclosure for sound direction
- Cable management

## 3D Printing

### Slicing for 3D Printer

1. **Download STL files** from Meca/3D folders
2. **Open in slicing software**:
   - Cura (Creality)
   - PrusaSlicer (Prusa)
   - Ultimaker Cura
3. **Recommended Settings**:
   - Layer Height: 0.2 mm
   - Infill: 20%
   - Support: Yes (enable for overhangs)
   - Print Speed: 50-60 mm/s
4. **Export G-code** and print

### Material Recommendations

- **PETG**: Good strength and flexibility
- **PLA**: Easiest to print, good detail
- **TPU**: For flexible parts (gaskets, clips)
- **ASA**: For outdoor/durable parts

### Print Time & Cost

Typical parts:
- Head case: 8-12 hours, ~100-150g filament
- Small brackets: 1-2 hours, ~10-20g
- Total assembly: 20-30 hours, ~300-500g

### Post-Processing

1. Remove support material
2. Sand surfaces if needed (120-200 grit)
3. Paint or coat for protection
4. Assemble mechanical parts

## Assembly Guide

### Before Assembly

1. Gather all printed parts
2. Collect hardware:
   - M3 bolts/screws/washers
   - M2.5 inserts (for threaded holes)
   - Cable clips (3M adhesive strips)
   - Double-sided tape (for components)

3. Prepare electronics:
   - Solder/solder headers to components
   - Prepare microphone I2S cables
   - Test all components individually

### Head Assembly Example

1. **Mount Display**
   ```
   OLED Housing
       ↓
   Insert OLED display
       ↓
   Clip/glue lens cover
   ```

2. **Mount Microphone**
   ```
   Head case
       ↓
   Microphone housing (glue with epoxy)
       ↓
   Connect I2S cables
   ```

3. **Mount Servo**
   ```
   Servo motor
       ↓
   Servo bracket (screw M3x8)
       ↓
   Connect to head case
   ```

4. **Install Cable Clips**
   ```
   OLED cable  → Clip path through head
   Mic cables  → Route to back
   Servo cable → Connect to ESP32
   ```

## Mechanical Design Principles

### Center of Gravity

- Place heavier components (battery) low and centered
- Counterbalance servos with weight distribution
- Ensures stability when moving

### Cable Management

- Use cable clips every 5-10 cm
- Avoid sharp bends (minimum 5mm radius)
- Group similar cables (I2S, power, control)
- Label cables for easier assembly

### Servo Mounting

- Mount on 90° brackets for head movement
- Use high-quality servo horn (metal preferred)
- Ensure smooth movement range
- Add endstop limiters if needed

### Thermal Management

- Ensure ESP32 has air circulation
- Keep battery away from heat sources
- Add small heatsinks if needed
- Monitor temperature during operation

## CAD Customization

### Modifying STL Files

1. **Open in CAD software**:
   - Fusion 360 (free for students/personal)
   - Tinkercad (browser-based, simple)
   - FreeCAD (open source)
   - Blender (3D modeling)

2. **Make changes**:
   - Modify dimensions
   - Add mounting holes
   - Adjust tolerances
   - Add custom features

3. **Export back to STL**
4. **Slice and print**

### Design Tips

- Keep minimum wall thickness: 1.5-2 mm
- Use 3-4 mm for mounting holes
- Add margin: Actual size = Model size + 0.2 mm
- Plan for assembly tolerance

## Weight Distribution

```
     Front of Robot
         ↓
    ┌─────────────┐
    │   Display   │ (50g)
    │   (OLED)    │
    └─────────────┘
         ↓
    ┌─────────────┐
    │ Microphone  │ (20g)
    └─────────────┘
         ↓
    ┌─────────────┐
    │   Servos    │ (200g x 2)
    └─────────────┘
         ↓
    ┌─────────────┐
    │  ESP32      │ (30g)
    │ + Battery   │ (150g)
    │ + Amplifier │ (50g)
    └─────────────┘
     Back of Robot
```

## Purchasing Hardware

### Electronics Suppliers

- **AliExpress**: Cheap components (slow shipping)
- **Amazon**: Fast shipping, higher price
- **Local Electronics Stores**: Best for quick needs
- **Specialized Suppliers**: Quality guaranteed

### Cost Estimation

| Component | Quantity | Cost |
|-----------|----------|------|
| ESP32 | 1 | $15-30 |
| OLED Display | 1 | $10-15 |
| INMP441 Mic | 1 | $5-10 |
| MAX98357A Amp | 1 | $5-10 |
| Servo Motors | 2-3 | $30-60 |
| 3D Printing | Filament | $20-40 |
| Wiring/Connectors | Bundle | $10-20 |
| **Total** | | **$100-200** |

## Advanced: CNC Machining

For more permanent/professional builds:

1. **Export to DXF** (2D profiles)
2. **Export to STEP** (3D CAD)
3. **Send to makerspace** or CNC shop
4. **Materials**: Aluminum, acrylic, wood

Cost is higher but results are more durable.

## Testing Physical Assembly

1. **Mechanical Test**
   - Check all joints move smoothly
   - Verify stability
   - Test cable routing

2. **Electrical Test**
   - Verify no shorts
   - Test connectivity
   - Check power distribution

3. **Functional Test**
   - Test servo movement
   - Verify display shows correctly
   - Check microphone audio input
   - Test speaker output

## Troubleshooting Assembly

### Parts Don't Fit

- Check STL scaling (should be in mm)
- Verify print settings (might have shrinkage)
- Sand down if necessary
- Adjust tolerances in CAD

### Cables Too Short

- Route through existing clips differently
- Add cable extenders (JST connectors)
- Reprint with longer cable ducts

### Servo Won't Move

- Check servo horn isn't hitting frame
- Verify servo mounting angle
- Add endstop limiters
- Check electrical connection

### Display Not Visible

- Check correct orientation
- Verify lens isn't scratched
- Adjust mount angle
- Clean lens

## Customization Ideas

1. **Add microSD Card Slot** - For audio logging
2. **Add Push Button** - For manual mode
3. **Increase Servo Count** - For more movement
4. **Add Battery Compartment** - Easier swapping
5. **Modular Design** - Swap parts easily

## Next Steps

1. Download STL files from Meca/3D
2. Slice and print parts
3. Gather hardware components
4. Assemble following guides above
5. Test mechanical movement
6. Install electronics
7. Upload firmware and test

## Resources

- **Tinkercad**: https://www.tinkercad.com (free CAD)
- **Fusion 360**: https://www.autodesk.com/products/fusion-360
- **FreeCAD**: https://www.freecadweb.org
- **Cura Slicer**: https://ultimaker.com/software/ultimaker-cura
- **Thingiverse**: https://www.thingiverse.com (browse designs)

---

Last updated: December 2025
