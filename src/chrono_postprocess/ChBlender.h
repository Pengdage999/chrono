// =============================================================================
// PROJECT CHRONO - http://projectchrono.org
//
// Copyright (c) 2014 projectchrono.org
// All rights reserved.
//
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file at the top level of the distribution and at
// http://projectchrono.org/license-chrono.txt.
//
// =============================================================================
// Authors: Alessandro Tasora 
// =============================================================================

#ifndef CHBLENDER_H
#define CHBLENDER_H

#include <fstream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <unordered_map>

#include "chrono/assets/ChVisualShape.h"
#include "chrono/physics/ChSystem.h"
#include "chrono_postprocess/ChPostProcessBase.h"

namespace chrono {
namespace postprocess {

/// Class for post processing implementation that generates scripts for Blender.
/// The script can be used in Blender to render photo-realistic animations, if the chrono_import.py add-on is 
/// installed in Blender. 
class ChApiPostProcess ChBlender : public ChPostProcessBase {
  public:
    ChBlender(ChSystem* system);
    ~ChBlender() {}

    /// Modes for type of vector arrow length 
    enum class ContactSymbolType {
        NONE = 0,
        VECTOR,
        SPHERE,
    };

    /// Modes for type of vector arrow length 
    enum class ContactSymbolVectorLength {
        CONSTANT = 0,
        ATTR,
    };

    /// Modes for type of vector arrow width
    enum class ContactSymbolVectorWidth {
        CONSTANT = 0,
        ATTR,
    };

    /// Modes for type of dot size
    enum class ContactSymbolSphereSize {
        CONSTANT = 0,
        ATTR,
    };

    /// Modes for colorizing vector or dot 
    enum class ContactSymbolColor {
        CONSTANT = 0,
        ATTR,
    };

    /// Add a ChPhysicsItem object to the list of objects to render.
    /// An item is added to the list only if it has a visual model.
    void Add(std::shared_ptr<ChPhysicsItem> item);

    /// Remove a ChPhysicsItem object from the list of objects to render.
    void Remove(std::shared_ptr<ChPhysicsItem> item);

    /// Add all ChPhysicsItem objects in the system to the list of objects to render.
    void AddAll();

    /// Remove all ChPhysicsItem objects that were previously added.
    void RemoveAll();

    /// Attach custom Blender commands to the given physics item.
    /// The provided string will be inserted as-is in the assets file corresponding to the physics item. Only one
    /// commands string can be attached to any physics item; a call to this function replaces any existing commands.
    void SetCustomCommands(std::shared_ptr<ChPhysicsItem> item, const std::string& commands);

    /// Set the path where all files (xxx.assets.py, output/state00001.py etc) will be saved.
    /// The path can be absolute, or relative to the .exe current path.
    /// Note that the directory must be already existing.
    /// At the execution of ExportScript() it will create files & directories like
    ///    base_path
    ///       exported.assets.py
    ///       anim
    ///          pic0000.bmp
    ///          ....
    ///       output
    ///          state00000.py
    ///          state00000.dat
    ///          state00001.py
    ///          state00001.dat
    ///          ....
    void SetBasePath(const std::string& mpath) { base_path = mpath; }

    /// Set transformation from Chrono frame to Blender 
    void SetBlenderFrame(const ChFrame<> mframe) {
        this->blender_frame = mframe;
    }
    /// Set transformation from Chrono frame to Blender (in Blender, Z is up axis). Default.
    void SetBlenderUp_is_ChronoY() {
        this->blender_frame = ChFrame<>(VNULL, Q_ROTATE_Y_TO_Z);
    }
    /// Set transformation from Chrono frame to Blender (in Blender, Z is up axis). Useful when using Chrono::Vehicle, based on SAE standard with Z up.
    void SetBlenderUp_is_ChronoZ() {
        this->blender_frame = ChFrame<>(VNULL, QUNIT);
    }

    /// Set the filename of the output Blender script.
    /// If not set, it defaults to "render_frames.assets.py".
    void SetOutputScriptFile(const std::string& filename) { out_script_filename = filename; }

    /// Set the filename of the image files generated by Blender.
    /// If not set, it defaults to "pic".
    void SetPictureFilebase(const std::string& filename) { pic_filename = filename; }

    /// Set the picture width and height - will write this in the output .ini file.
    void SetPictureSize(unsigned int width, unsigned int height) {
        picture_width = width;
        picture_height = height;
    };

    /// Set the default camera position and aim point - will write this in the output .assets.py file.
    void SetCamera(ChVector<> location, ChVector<> aim, double angle, bool ortho = false);

    /// Set the default light position and color - will write this in the output .assets.py file.
    void SetLight(ChVector<> location, ChColor color, bool cast_shadow);

    /// Set the background color - will write this in the output .assets.py file.
    void SetBackground(ChColor color) { background = color; }

    /// Turn on/off the display of the COG (center of mass) of rigid bodies.
    /// If setting true, you can also set the size of the symbol, in meters.
    void SetShowCOGs(bool show, double msize = 0.04);

    /// Turn on/off the display of the reference coordsystems of rigid bodies, particle in particle clouds, etc..
    /// If setting true, you can also set the size of the symbol, in meters.
    void SetShowItemsFrames(bool show, double msize = 0.05);

    /// Turn on/off the display of the reference coordsystems of each asset shape instance.
    /// If setting true, you can also set the size of the symbol, in meters.
    void SetShowAssetsFrames(bool show, double msize = 0.03);

    /// Turn on/off the display of the reference coordsystems for ChLinkMate constraints.
    /// If setting true, you can also set the size of the symbol, in meters.
    void SetShowLinksFrames(bool show, double msize = 0.04);

    /// Turn off the display of contacts
    void SetShowContactsOff();

    /// Turn on the display of contacts, using arrows to show vectors. 
    /// The lenght of the arrow can be: constant, or force strength multiplied by 'scale_length', or attribute multiplied by 'scale_length'.
    /// The width of the arrow can be: constant, or force strength multiplied by 'scale_length', or attribute multiplied by 'scale_length'.
    void SetShowContactsVectors(
                        ContactSymbolVectorLength length_type,
                        double scale_length,          // if ContactSymbolVectorLength::CONSTANT means abs.length, otherwise is scaling factor for attr
                        const std::string scale_attr, // needed if ContactSymbolVectorLength::ATTR, options: 'norm'. Otherwise "" 
                        ContactSymbolVectorWidth width_type,
                        double scale_width,           // if ContactSymbolVectorWidth::CONSTANT means abs.width, otherwise is scaling factor for norm or attr
                        const std::string width_attr, // needed if ContactSymbolVectorWidth::ATTR, options: "norm". Otherwise ""
                        ContactSymbolColor color_type,
                        ChColor const_color,          // if ContactSymbolColor::CONSTANT, otherwise not used
                        const std::string color_attr, // needed if ContactSymbolColor::ATTR, options: "norm". Otherwise "" 
                        double colormap_start, // falsecolor start value, if not  ContactSymbolColor::CONSTANT,
                        double colormap_end, // falsecolor start value, if not  ContactSymbolColor::CONSTANT
                        bool do_vector_tip = true
                         );

    /// Turn on the display of contacts, using spheres to show contact locations. 
    /// The size of the arrow depends on force strength multiplied by 'scale_length'.
    void SetShowContactsSpheres(
                         ContactSymbolSphereSize size_type,
                         double scale_size,    // if ContactSymbolSphereSize::CONSTANT means abs.size, otherwise is scaling factor for norm or attr
                         ContactSymbolColor color_type,
                         ChColor const_color,  // if ContactSymbolColor::CONSTANT, otherwise not used
                         double colormap_start, // falsecolor start value, if not  ContactSymbolColor::CONSTANT,
                         double colormap_end, // falsecolor start value, if not  ContactSymbolColor::CONSTANT
                         const std::string size_attr = "", // needed if ContactSymbolSphereSize::ATTR
                         const std::string color_attr = ""  // needed if ContactSymbolColor::ATTR
                         );

    /// Set thickness for wireframe mode of meshes.
    /// If a ChTriangleMeshShape asset was set as SetWireframe(true), it will be rendered in Blender as a cage of thin
    /// cylinders. This setting sets how thick the tubes.
    void SetWireframeThickness(const double wft) { wireframe_thickness = wft; }
    double GetWireframeThickness() const { return wireframe_thickness; }

    /// Set a string (a text block) of custom Blender commands that you can optionally
    /// append to the Blender script file, for example adding other lights, materials, etc.
    /// What you put in this string will be put at the end of the generated script, to be
    /// executed each time the menu File/Import/Chrono import is called.
    void SetCustomBlenderCommandsScript(const std::string& text) { custom_script = text; }
    const std::string& GetCustomBlenderCommandsScript() const { return custom_script; }

    /// Set a string (a text block) of custom Blender commands that you can optionally append to 
    /// the Blender script files that are load at each timestep, e.g.,
    ///    state00001.py, state00002.py,
    /// for example adding other lights, materials, etc.
    void SetCustomBlenderCommandsData(const std::string& text) { custom_data = text; }
    const std::string& GetCustomBlenderCommandsData() const { return custom_data; }

    /// When ExportData() is called, it saves .dat files in incremental way, starting from zero: 
    ///    data00000.dat, data00001.dat etc.,
    /// but you can override the formatted number by first calling SetFramenumber().
    void SetFramenumber(unsigned int fn) { framenumber = fn; }

    /// Export the script that will be used by Blender that contains the definitions of assets of 
    /// geometric shapes, lights, etc.
    /// This function must be called once at the beginning of the animation.
    void ExportScript() { ExportScript(out_script_filename); }

    /// As ExportScript(), but overrides the filename.
    virtual void ExportScript(const std::string& filename) override;

    /// This function must used at each timestep to export the state of the shared assets as 
    /// file(s) with incremental numbering in output/
    /// The user should call this function in the while() loop of the simulation, once per frame.
    void ExportData();

    /// As ExportData(), but overrides the automatically generated filename. Prefer using ExportData() so naming is automatic.
    virtual void ExportData(const std::string& filename) override;

    /// Set if the assets for the entre scenes at all timesteps must be appended into one
    /// single large file "exported.assets.py". If not, assets will be written inside
    /// each state00001.dat, state00002.dat, etc files; this would waste more disk space but 
    /// would allow assets whose settings change during time (ex time-changing colors)
    void SetUseSingleAssetFile(bool use) { single_asset_file = use; }

  private:
    void UpdateRenderList();
    void ExportAssets(ChStreamOutAsciiFile& assets_file, ChStreamOutAsciiFile& state_file);
    void ExportShapes(ChStreamOutAsciiFile& assets_file, ChStreamOutAsciiFile& state_file, std::shared_ptr<ChPhysicsItem> item);
    void ExportMaterials(ChStreamOutAsciiFile& mfile,std::unordered_map<size_t, std::shared_ptr<ChVisualMaterial>>& m_materials, const std::vector<std::shared_ptr<ChVisualMaterial>>& materials, bool per_frame);
    void ExportItemState(ChStreamOutAsciiFile& state_file,
                       std::shared_ptr<ChPhysicsItem> item,
                       const ChFrame<>& parentframe);

    /// List of physics items in the rendering list.
    std::unordered_set<std::shared_ptr<ChPhysicsItem>> m_items;

    /// Association between a physics item and a string of custom Blender commands.
    std::unordered_map<size_t, std::string> m_custom_commands;

    std::unordered_map<size_t, std::shared_ptr<ChVisualShape>> m_blender_shapes;        ///< cache of visual shapes
    std::unordered_map<size_t, std::shared_ptr<ChVisualMaterial>> m_blender_materials;  ///< cache of visual materials
    std::unordered_map<size_t, std::shared_ptr<ChCamera>> m_blender_cameras;            ///< cache of cameras

    std::unordered_map<size_t, std::shared_ptr<ChVisualShape>> m_blender_frame_shapes;        ///< cache of visual shapes, mutable (reset each frame)
    std::unordered_map<size_t, std::shared_ptr<ChVisualMaterial>> m_blender_frame_materials;  ///< cache of visual materials, mutable (reset each frame)


    std::string base_path;
    std::string pic_path;
    std::string out_path;
    std::string pic_filename;

    std::string out_script_filename;
    std::string out_data_filename;

    unsigned int framenumber;

    ChFrame<> blender_frame;

    bool camera_add_default;
    ChVector<> camera_location;
    ChVector<> camera_aim;
    ChVector<> camera_up;
    double camera_angle;
    bool camera_orthographic;
    bool camera_found_in_assets;

    ChVector<> def_light_location;
    ChColor def_light_color;
    bool def_light_cast_shadows;

    bool COGs_show;
    double COGs_size;
    bool frames_item_show;
    double frames_item_size;
    bool frames_asset_show;
    double frames_asset_size;
    bool frames_links_show;
    double frames_links_size;

    ContactSymbolType contacts_show ;
    double contacts_maxsize;
    ContactSymbolVectorLength contacts_vector_length_type;
    std::string contacts_vector_length_attr;
    double contacts_vector_scalelenght;
    ContactSymbolVectorWidth  contacts_vector_width_type;
    std::string contacts_vector_width_attr;
    double contacts_vector_scalewidth;
    ContactSymbolSphereSize  contacts_sphere_size_type;
    std::string contacts_sphere_size_attr;
    double contacts_sphere_scalesize;
    ContactSymbolColor  contacts_color_type;
    std::string contacts_color_attr;
    ChColor contacts_color_constant;
    double  contacts_colormap_startscale;
    double  contacts_colormap_endscale;
    bool contacts_vector_tip;

    double wireframe_thickness;
    ChColor background;

    unsigned int picture_width;
    unsigned int picture_height;

    std::string custom_script;
    std::string custom_data;

    bool single_asset_file;
};

}  // end namespace postprocess
}  // end namespace chrono

#endif
