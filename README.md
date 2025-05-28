<a href="https://freecad.org"><img src="/src/Gui/Icons/freecad.svg" height="100px" width="100px"></a>

### Your own 3D Parametric Modeler

Warning
-------

**This branch is a moving target!** Do not use it as base for anything!

Commits list
------------
```
[ ] 9374dcfc9e App: restore previous value of specular color for default material
[x] 7c45a0bed1 Measure: Fix crash in Measurement::length() when selecting infinite edge (e.g. PD datum line) (#23291)
[x] 7204b0d34a Measure: Implement quick measure as command and make it toggable (#23291)
[-] b67e298204 Fix German translation
[x] e4041a79c6 Gui: Add Action::setBlockedChecked (#21659)
[ ] 214c8fcd9f Gui: By default disable Python profiler and set timeout to 1000 ms
[x] 75c425343e Gui: Use Py::SmartPtr instead of raw pointer (#21659)
[ ] df7a7f8d91 Part: Implement extension to add physical properties at runtime
[ ] d0397124c9 RE: Improve parameter correction for B-Spline surface fitting
[x] e410a891ca Base: Imporove Python exception handling (#21693)
[x] d36e501061 App: Handle possibly raised exception in ZipFile constructor (#21706)
[ ] 91a5d2a059 Part: When saving shape in binary mode continue even if it's empty
[ ] 7709d30ad0 Part: Fix correct usage of BRepIntCurveSurface_Inter
[ ] 2136739a70 Part: Inside TopoShape::makeElementPrismUntil use more points to check for concavity
[x] 1f7f118b4d PD: Positive offset on a binder shouldn't turn holes into squares (#21713)
[ ] d12e248ff0 Part: Expose BRepOffsetAPI_MakeOffset to Python
               ^- needs revisit!
[ ] 94e26b8eb9 Sketcher: Fix const-correctness and possible integer underflow
[-] 957de0a7cf Gui: Show tooltips even when window is not in focus (#19154)
[x] bd7d8f1987 Mesh: Fix crash MeshPy::setPoint & MeshPy::movePoint (#23213)
[ ] 85aae9e35a Mesh: Add test cases for setPoint & movePoint
[x] e812045297 Core: Convert transparency to alpha value for old project files (#24891)
[ ] 71acf54fba [skip ci] Gui: Fix pasting if target object comes from XLink document
[ ] 726517a411 Build: Set active branch name
[ ] 60280a0734 Import: Use ImportGui module for IGES import & export
[x] 925858d5e7 Gui: Fix undo/redo behaviour in transformation tool (#21659)
[-] 095726f179 Part: Fix regression in TopoShape::makeElementCopy (#19233)
[ ] da6d1636ca Tests: Unit test to demonstrate the fix of TopoShape::makeElementCopy
[ ] 5a865994d0 Part: Fix plane utility for empty sketches
[x] 07e6a6356d Test: Do not write test files into CWD (#21906)
[ ] 7039ba8835 Gui: Fix linking error on Linux by not defining _USE_3DCONNEXION_SDK
[ ] da1f3b1101 Part: Fix correct loading of older project files
[ ] e9cda10299 Modules: The modules' show() function should return the created feature
[-] d22ce55061 Part: Revert questionable change in Part::Feature::onChanged (#26992)
[-] a9100c1425 PD: Fix loft between curved faces (#22165)
[x] e832dd48ed PD: Show commands in task panel when selecting (sub) shape binders (#21713)
[x] 5e3e0f7e37 PD: Restore GUI commands for PartDesign datum objects (#21713)
[ ] 5af9ee5c89 PD: Fix regression in TaskDlgFeatureParameters::accept()
[x] f2117d4088 PD: Fix regression about single-solid check (#22169)
[x] 8961dea5ce PD: Use isSingleSolidRuleSatisfied() for pipe feature (#22169)
[x] aed47a2330 Test: Add test case for UnitsSchemaMeterDecimal (#21147)
[ ] f78e3a0679 Sketch: Make option 'ESC can leave Sketch edit mode' working again
[x] 5b3b09636d Gui: Adjust transform dragger after undo/redo (#21659)
[ ] 404cef183f Mod: Fix several memory leaks
[x] a40610fdc6 PD: Correctly handle single solid rule for loft with and without base (#22169)
[x] 5917bcd46d PD: SubtractivePipe Fails (#21713)
[ ] e94ce8caba Sketch: Fix crash after creating B-Spline
[x] 4596f13c71 Test: Add further test cases for quantities (#21147)
[x] 301523d387 Gui: Fix several methods in Workbench to list all items (#21659)
[x] c6f4ac5e0c Gui: Fix DlgPreferencesImp::minimumDialogWidth() to return a reasonable width (#21659)
[ ] 60cc44d33a Import: Fix import/export of STEP file
[x] a9dd398103 PD: Fix support of datum lines in draft and polar pattern (#21713)
[x] 442aa4ad31 PD: Support of plane in linear pattern feature (#21713)
[ ] 51f6c7a031 Part: Fix TopoShape::makeElementMirror
[ ] 30706898c8 Part: Adjust Mirroring::execute()
[x] 6b4989bbb5 Gui: Do not round color values set in property editor (#21659)
[ ] c18303e5c9 Gui: Fix shading after leaving a sketch from edit mode
               ^- mainline claims to fix that, but it seems rather mitigated...
[x] b576dbcef7 Gui: Make AccelLineEdit a subclass of QKeySequenceEdit (#21659)
[x] a79fb58175 Mesh: Refactor and optimize ReaderOBJ (#23214)
[x] e5a51f2ebb Measure: Fix several crashes in Measurement (#23291)
[ ] 97c182570d Part: Fix Extrusion::calculateShapeNormal
[x] 14c3f2eef9 Gui: Fix command StdCmdProperties (#21659)
[ ] b7f34870ef Base: Expose PyObjectBase type to Python with helper functions
[ ] d64274a4d6 Part: Optimize ViewProviderPartExt::updateData
[ ] 720a866402 PD: Fix crash in Loft::getSectionShape
[ ] c6161ec437 Sketch: Fix crash when moving sketch with hidden constraints
[ ] dcd899ca0a Sketch: Add unit test for issue 19566
[x] 21e9930df3 Gui: Fix crash when trying to transform link object (#21659)
[-] 94dc2ade6e Part: Fix FCBRepAlgoAPI_BooleanOperation::Build
[ ] 696fca2282 Part: Fix multi-common feature
[-] 19a855fae6 TD: Fix regression in DrawViewSection::makeSectionCut (#23102)
[x] 467e1bdb9c PD: Improve revolution feature (#21713)
[x] 0b99f2e29e PD: Improve groove feature (#21713)
[x] 52e6e3f6df PD: Move common code of Revolution and Groove to base class (#25607)
[x] 0213746de0 PD: Clean-up revolution & groove code (#25607)
[x] f62cd26af3 TD: Fix crash on selecting 3D edge for dimension (#23101)
[-] a8cb46bcbf Fix build failure
               ^- alternate change upstream
[-] bb6f88c319 Gui: Allow to open links in license tab
               ^- alternate change upstream
[x] 6d6b6fe2b8 Gui: Fix stackoverflow when loading corrupted file (#21659)
[-] 6fc9387aa2 Gui: Add 'Open file location' command to context-menu of tree view
               ^- alternate change df2c6637e634 upstream
[ ] 00200a80f6 Gui: Simplify and fix code for ifcopenshell version check
               ^- now moved to App
[x] ec6fef7daa Gui: Use QDialogButtonBox in DlgExpressionInput (#21659)
[ ] d2ab47ef32 Part: Fix unicode issue in TaskCheckGeometryResults
[-] ba4208605b Part: Fix slice feature (#25293)
[ ] bf7b370e05 App: In ExpressionParser::parse add the failing expression text to the error message
[ ] 7507da8433 App: Refactor PropertyExpressionEngine::afterRestore()
[-] c1aa39f6be Start: Clean-up code
[x] 42742c9a71 Start: Fix possible crash when opening Start page (#21706)
[ ] cd798605f8 Start: Fix flashing widget of Start page
[ ] 48c3610199 App: Fix several lint warnings
[!] 9a61380df2 Part: Fix stack overflow in FCBRepAlgoAPI_BooleanOperation
[-] aee0d55bd6 BIM: Disable broken unit test (#21541)
[ ] 063e891484 Test: Add test case for issue 19980
[-] 9f26c4ab16 App: Fix possible crash in Document::removeObject() (#21481 ???)
[!] bff735b07d App: Fix clang-tidy warnings
[-] 026fcf29de Sheet: Disable zooming (#20786)
[-] 4bbb728c4f TD: Fix element mismatch in Vertex::Save (#21374)
[-] 7a017a60be Harmonize with upstream
[x] 347e2d2b22 Sheet: Fix reading xlsx files (#21785)
[-] e761462414 App: Fix PropertyLink::resetLink() (#20223)
[!] ae462ef4c5 App: Minor tweak
[x] a7882c6cbb Assembly: Fix crash in ViewProviderAssembly::findDragMode (#25678)
[ ] 8804030073 Sketcher: Handle exception in DrawSketchHandlerOffset::drawOffsetPreview
[-] b61b946b4a Assembly: Fix crash in AssemblyLink::onChanged (#24198)
[-] f37e1bc301 Part: Handle exception in CrossSections::apply() (#23090)
[ ] abab77885a Part: In CrossSection class switch back to OCC's boolean operation base classes
[ ] b36e679eb5 App: In ProjectFile::loadDocument() check for parse errors
[x] f641e5ed19 Gui: Improve document recovery (#21659)
[ ] 1683238e36 App: Improve handling with freeze status
[ ] 6bb8938e75 App: Remove assert() from PropertyXLink::Restore
[ ] d1cccfdfd0 App: Refactor Document::writeObjects
[ ] e11d4cd134 Base: Add C++ iostate methods to Writer class
[ ] 88f4e1f64b App: Improve Document::writeObjects
[ ] 873572c2f3 Support of OCCT 7.9.0
[x] f79be10e3a Gui: Fix crash in Command::keySequenceToAccel (#21659)
[ ] 3463d828a1 Part: Fix freeze when trying to tessellate broken face
[ ] 3e719d8ada App: Add convenience methods to DocumentT and DocumentObjectT
[ ] 94b718f809 App: Fix some linter warnings
[!] 47f51f0760 Sketch: Clear TaskSketcherElements and TaskSketcherConstraints when deleting document or sketch
[ ] 0b7987d8d3 Sketch: Fix crash in EditModeGeometryCoinManager::updateGeometryColor
[ ] a18943f4bf Sketch: Improve exception handling
[x] c2ecfb833b Gui: Handle exception when trying to start editing an object (#21659)
[x] c9d35d527e Sheet: Fix crash in Cell::setContent (#21785)
[-] 58df18566a App: Fix macOS starting problem
[x] fbc361947d Gui: Replace calls of FCMD_SET_EDIT with Gui::cmdSetEdit (#21659)
[ ] c724eee495 Gui: Remove poorly conceived user edit modes
[-] 2dbbc248fb CMake: Remove FREECAD_USE_PYBIND11 as a build option
[-] 2cef669eac TD: Remove [[deprecated]] attribute to fix compiler warning (#23101)
[x] 9d39c29148 Gui: avoid code duplication in PythonEditor (#21659)
[x] b96a786625 Sketcher: Better fix for issue 12925 (#22417)
[x] 3e578cb68a Gui: Fix crash when creating a LCS (#21924)
[ ] 80c61e1bf7 Sketch: Refactor SketchObject::getPoint
[!] 1a9c395b0c Sketch: Refactor SketchObject::exposeInternalGeometry
[x] 1f308838f5 Sheet: Improve handling of unsupported formulas (#21785)
[ ] 456d0cb094 Part: Add transaction support to section cutting dialog
[x] f03f9ed51e Measure: Fix TaskMeasure (#23291)
[-] 370f5dfad0 Core: Inform user if while saving the project a relevant system error has occurred.
               ^- alternate change upstream
[x] 64ffcc9bee Base: Allow to also throw exception by given Python type (#21693)
[!] c9911e3981 Core: Exception issues
[ ] aa59238882 Base: Update license header in FileTemplate
               ^- dropped
[ ] a785ac50ff Core: Refactor code to install new and SIGSEGV handlers
[ ] 8ec6cc89f1 Part: Use signal handlers based on OCC's implementation
[-] 20b3dceca5 [skip ci] Part: Use Part::SignalException in FCBRepAlgoAPI_BooleanOperation::Build
[ ] b7de55d398 Gui: Fix i18n support of MeshInfoWatcher
[ ] 6472760d23 BIM: Disable broken unit test
[-] ab5c30f2d8 Draft: Sync with upstream
[ ] b1526619a9 [skip ci]TD: Don't cast time_t to a potentially 32-bit type
[ ] bd27b1bd4d Core: Add SequencerBase::stopStep() and SequencerLauncher::stop()
[ ] e0fb1b1df2 Part: Port ProgressIndicator to OCCT >= 7.5
[ ] e44e6afd36 Part: Implement ProgressIndicator with progress dialog
[ ] f171fae137 Import: Use progress bar for STEP, IGES and glTF import
[-] 8118d84a76 Fixing ambiguiguous python shebang in installed script freecad-thumbnailer
[!] b934196b7a Material: Move definition of exception classes to source file
[ ] 50c7ea5f66 MSVC: Fix warnings & build failure
[ ] eaa04d686d Mod: Check for Python commands before adding to menu
[x] 76a4ec48ea Base: Remove weird error parameter from encode (#21693)
[x] 5d2139856e Gui: Fix crash in ~PythonBaseWorkbench with Python 3.12 (#21659)
[ ] def64a7e84 Part: Replace deprecated Handle_ with Handle()
[ ] 03c589fb12 Part: Improve TopoShape::findPlane
[!] 56a19f18e3 App: Reduce 'using namespace' statements
[ ] 075267c76a MinGW: For Qt6 use QtUiTools library
[ ] e643884193 MeshPart: Fix possible crash in mesh flattening
[ ] aeedd236d5 Fix macro Q_DISABLE_COPY_MOVE
[ ] 194bc86b25 App: Allow to get a label for a property
[ ] ab143f2f37 PD: Reimplement Pipe::getPropertyLabel() to handle typos in property names
[ ] 7007235cbd Gui: Allow to show label in property editor
[-] 4cdf67ca1b PD: Fix typos
[x] b2c9d8a4a9 TD: Fix loading project files (#23101)
[-] 74a637e104 Use Base::toRadians() instead of manually converting
[-] b018459ad6 Use Base::toDegrees() instead of manually converting
[x] 2530130523 Measure: Extend area measurements (#23291)
[x] 13dafed175 Measure: Extend area measuremnts (#23291)
[ ] 0c58741caf Material: Fix several issues
               ^- dropped, need to try harder resolving conflicts (note: Measurement...)
[x] 6da79aa39f Measure: Improve TaskMeasure dialog (#23291)
[x] 1d86f8ffb8 Measure: Fix some linter warnings (#23291)
[ ] d299612e10 Sketch: Fix heap-buffer-overflow
[ ] b2bcd37eff Sketch: Add unit test for periodic spline
[ ] c124d77681 Sketcher: Make command CmdSketcherArcOverlay toggable
[-] 99c246436a Revert "CI: Update to LibPack 3.1.1"
[ ] 218d3fa857 Part: Use higher tolerance in getCameraAlignmentDirection to check for planar faces
[x] 7111ad261b Gui: Fix TreeWidget::addDependentToSelection (#21679)
[ ] f652184407 Gui: Refactor code to get program information
[x] 952787f2ed Gui: Implement navigation style for NX (#21813)
[x] 314fd03e3f Gui: Refactor navigation styles (#21813)
[x] b5981b0189 Gui: Refactor navigation styles (#21813)
[x] efd7459f3a Gui: Refactor navigation styles (#21813)
[ ] d26e496d2d Base: Add class ParameterObserver
[ ] ed46ee5b9e Fix Mingw build
[ ] 95f42bdf61 Gui: Set minimum width to tree widget of actions panel
[!] a2d635b77a Core: Add keyword 'locked' to addProperty() of DocumentPy, DocumentObjectPy and ViewProviderPy
[ ] f3792b1c6c [skip ci] Gui: Minor code cleanup
[!] d078e970d6 [skip ci] App: Simplify creation of string from array  in processProgramOptions
[ ] fb97826612 App: drop support of 'AutoNameDynamicProperty' in DynamicProperty::addDynamicProperty
[ ] dea76378bf Gui: Fix linter warnings & cleanup code
[x] 6da2f356e4 Gui: Move define HAS_QTBUG_129596 to separate header file (#21659)
[x] 09e164acfe Gui: Improve auto-saving (#21659)
[ ] d3862effce App: Fix PropertyFileIncluded::SaveDocFile
[ ] f8a5974f81 App: Add overloaded method Document::removeObject
[ ] c40f6f8336 App: Add overloaded method Application::closeDocument
[ ] 416566bae7 App: Move Python functions from Application to ApplicationPy
[ ] 3eecd1afa6 App: Fix linter warnings
[ ] 6472da46e7 CAM: Change CommandPy::representation() to create a new Command object from its output
[ ] 542a83011d CAM: Fix and code cleanup in CommandPy
[ ] fe889a1e8f Base: remove explicit namespace-name qualifier from *PyImp.cpp
               ^- dropped, need to try harder resolving conflicts (after units merge)
[x] fd49803617 Assembly: Improve const correctness (#25678)
[ ] c3f66a3229 App: Replace C macros with lambda expression
[x] 3f159c5857 Gui: In DockWindowItems::addDockWidget replace the booleans with the enum class DockWindowOptions (#21659)
[x] 6ffe81dd54 Helper function to get preferred delete key depending on platform (#21844)
[-] ae21c657af Measure: Fix visibility issues (#23145)
[ ] 6b0f969a3c Part: Implement ShapeAttributeProvider
[ ] 910914a1a9 Part: Implement PartPseudoShapeProvider
[ ] a9e0fca883 Base: Create a quantity of value 0.0 instead of DOUBLE_MIN from an empty string
               ^- dropped, need to try harder resolving conflicts (after units merge)
[ ] 8de005716a App: Optimize PropertyMaterialList::getDiffuseColors()/getTransparencies()
[x] 5400021178 Simplify code using xerces namespace (#25673)
[ ] 2925cc2b76 Tests: If FREECAD_WARN_ERROR is set than also abort for compiler warnings in test code
[ ] 17ad6a13bc App: Fix crash in Transaction::addObjectChange
[ ] 3acc5cf0c2 Make double click consitent to context menu of view providers
[!] a2696916a3 Material: Fix MaterialTreeWidgetPy
[!] b69fd82e33 Gui: Clean up code in DlgAddPropertyVarSet
[!] aa6f36de4e Gui: Fix i18n issues of  DlgAddPropertyVarSet
[x] ce523e64b5 Gui: Improve DlgAddPropertyVarSet (#21659)
[ ] 0e931dec39 SMESH: Disable code to delete output stream
[ ] 66cc0f23a3 Fem: Optimize netgen mesh creation
[-] 0dbd1d7b4d Move testDXFImportCPPIssue20195 from Document to Draft tests (#24317)
[!] ce04ba4e12 Improve inter-module dependencies
[ ] 3001042847 TD: Cleanup code
               ^- dropped, already upstream (huh?)
[x] 883750c77a Gui: Add helper function Command::getActiveDocument() to get active App document (#23100)
[x] d346f22646 Points: Add method PointKernel::moveGeometry (#23100)
[x] 1f340a72b9 Points: Optimize translation of imported points (#23100)
[ ] ca7b5b6c25 Sketch: Fix backward compatibility with old versions
[ ] ad318ecbbf FEM: Fix build failures/warnings
[ ] 0341aefa41 Draft: Fix _get_incmd_shortcut
[x] 2afc1d6573 Gui: Show date & time in locale specific format (#21659)
[ ] 6372b63191 Start: Add date of creation and modification in tool tips
[ ] c3e5346a5c Gui: Refactor CallTipsList
[ ] 2874465386 Gui: Fix linter warnings
[-] ca7e25c717 Gui: Apply clang-format to CallTipsList (#21019)
[ ] 432bff02f0 Gui: Show call tips in more complex cases
[ ] fd19f2360f Gui: Change order of items in call tips list
[ ] f545f37ca1 Use Application::getExecutableName() instead of Config()["ExName"]
[x] e0074862e5 Base::Matrix4D (#22015)
[x] fcb6fbef4b Base::Matrix4D (#22015)
[x] b79d1e9a18 Base: Simplify Base::Matrix4D (#22015)
[x] a48a0d01fd Base: Use nested std::array for Matrix4D class (#22015)
[x] ffef90c4e5 TD: Fix compiler warnings (#23101)
[x] 135f7f1294 PD: Increase the tolerance to check for planar surfaces (#23291)
[ ] f2b084172e [skip ci] Tools: import missing module
[x] c8024584a4 Measure: Extend angle measurements (#23291)
[!] a018eb74a6 Base: Fix linter warnings
[x] 6dc7aca74b [skip ci] Base: Fix typo (#21693)
[-] 59c6b7194a Base: Add numbers header
[-] a4073692ad Replace D_PI with numbers::pi
[-] 5fc5871253 Replace F_PI with numbers::pi_v<float>
[-] 32c66f7ef3 Remove unused DOUBLE_MIN
[-] 410b523300 Replace DOUBLE_MAX with std::numeric_limits<double>::max()
[-] 1df88aff42 Replace FLOAT_MAX with std::numeric_limits<float>::max()
[-] 738655f518 Replace FLT_* with std::numeric_limits
[!] b680f90706 Mesh: Replace defines with constexpr variables
[-] 812e6e9566 Replace DBL_* with std::numeric_limits
[-] fe5541dba5 Replace INT_* with std::numeric_limits
[!] 1ef03afd4e Mesh: Use an alias type for 'unsigned short'
[ ] b082d533cc Part: Fix crash in Revolution::execute()
[ ] 8b34770b4d Tools: Add ExpressionLineEdit to QtDesigner plugin
[x] db0b6c099d Gui: Implement validator for ExpressionLineEdit
[ ] b142ac3f22 [skip ci] PD: remove unneeded class member 'isApplying' from 'TaskHoleParameters'
[!] e9def3533e PD: Refactor TaskHoleParameters
[!] a6495bd2c0 PD: Fix linter warnings for TaskHoleParameters
[!] 0b4aef52ee PD: Change layout of control elements of TaskHoleParameters dialog
[!] f16c1c4b1d PD: Allow to create holes from sketch points
[-] 2e5283db2e [skip ci] PD: Update German translation
[x] 75c3b7ed2b Sheet: Further cleanup in Sheet class (#21785)
[ ] 82aef8b242 Gui: Refactor WorkbenchGroup::refreshWorkbenchList()
[ ] 35982be775 Gui: Do not allow to activate disabled workbench
[!] ce0bd4334b Sketch: Refactor SketchObject::buildShape()
[x] b403f4b5ea Surface: Add task panel for blending curve (#21825)
[x] 59d6ec4e1f Surface: Fix transaction support of BlendCurvePanel and bind widgets to properties (#21825)
[x] f84e886df4 [skip ci] Base: Fix size in array (#21693)
[ ] 4eac6f30e2 [skip ci] Gui: Suppress linter warnings
[x] c3178babfb Points: Replace defines with constexpr (#23100)
[ ] 44a7bf15f9 Improve handling of recent files list
[ ] 60eaa892e8 Gui: Separate code from data in class InputHintWidget
[ ] 14620e47d1 Gui: use std::variant to avoid duplication of enums
[-] fe4b8a311a Tests: Replace M_PI with numbers::pi
[-] 2d31402575 TD: Replace M_PI with numbers::pi
[-] 0c87787424 Core: Replace M_PI with numbers::pi
[-] aef8301f41 Part: Replace M_PI with numbers::pi
[-] 45676dbd9f Fem: Replace M_PI with numbers::pi
[-] c572109cf1 CAM: Replace M_PI with numbers::pi
[-] 5619e4bf8f Sketcher: Replace M_PI with numbers::pi
[ ] 4e737e9b59 Py3.13: Fix warning: missing field 'tp_versions_used' initializer
[ ] 67a783cdb6 Py3.13: Replace deprecated Py_GetPath() with PySys_GetObject("path")
[ ] d3661f0575 Py3.13: Replace deprecated PyWeakref_GetObject with PyWeakref_GetRef
[x] c34478b32b Py3.13: Update PyCXX to version 7.1.11 (#22856)
```
