#include <map>
#include <string>
#include <cstdio>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dxflib/dl_dxf.h>
#include <dxflib/dl_creationadapter.h>
#include <dxflib/dl_attributes.h>


class layerFilter : public DL_CreationAdapter
{
public:
  typedef std::map<std::string, DL_WriterA*> writer_map_type;

private:
  writer_map_type _writers;
  DL_Dxf* _dxf;

  static inline int mkdir_or_exist()
  {
    // create dir if it does not exist
    if ((mkdir("parts", 0700) == -1) && (errno != EEXIST)) return -1;
    return 0;
  }

  static inline const std::string make_partname(const std::string& name)
  {
    const std::string partname =
      std::string("parts/") + name + std::string(".dxf");
    return partname;
  }

  static bool is_part_layer(const std::string& layername)
  {
    for (unsigned int i = 0; i < layername.size(); ++i)
      if ((layername[i] < '0') || (layername[i] > '9'))
	return false;
    return true;
  }

  DL_WriterA* find_part_writer(const std::string& layername)
  {
    const writer_map_type::iterator i = _writers.find(layername);
    if (i == _writers.end()) return NULL;
    return i->second;
  }

  template<typename data_type>
  bool addCommon(const data_type& d, DL_WriterA*& w, DL_Attributes& a)
  {
    a = getAttributes();
    w = find_part_writer(a.getLayer());
    if (w == NULL) return false;
    a.setLayer(std::string("0"));
    return true;
  }

public:
  layerFilter(DL_Dxf* dxf)
  {
    _dxf = NULL;
    if (mkdir_or_exist() == -1) return ;
    _dxf = dxf;
  }

  virtual ~layerFilter()
  {
    // close all the part writers
    writer_map_type::iterator i;
    for (i = _writers.begin(); i != _writers.end(); ++i)
    {
      DL_WriterA* const writer = i->second;
      writer->sectionEnd();
      writer->dxfEOF();
      writer->close();
    }
  }

  inline bool has_failed() const
  {
    return _dxf == NULL;
  }

  virtual void addLayer(const DL_LayerData& d)
  {
    // ::printf("%s\n", __FUNCTION__);

    // skip if  not a part layer
    if (is_part_layer(d.name) == false) return ;

    // not found, new layer
    if (_writers.find(d.name) == _writers.end())
    {
      const std::string partname = make_partname(d.name);
      DL_WriterA* const writer = _dxf->out(partname.c_str(), DL_Codes::VER_2000);
      if (writer != NULL)
      {
	printf("new part: %s\n", d.name.c_str());

#if 0 // SW10 fails if header written
	_dxf->writeHeader(*writer);
	writer->sectionEnd();
#endif // SW10
	writer->sectionEntities();
	_writers.insert(writer_map_type::value_type(d.name, writer));

	DL_WriterA* w;
	DL_Attributes a;
	if (addCommon(d, w, a)) _dxf->writeLayer(*w, d, a);
      }
    }
  }

  virtual void addPoint(const DL_PointData& d)
  {
    DL_WriterA* w;
    DL_Attributes a;
    if (addCommon(d, w, a)) _dxf->writePoint(*w, d, a);
  }

  virtual void addLine(const DL_LineData& d)
  {
    DL_WriterA* w;
    DL_Attributes a;
    if (addCommon(d, w, a)) _dxf->writeLine(*w, d, a);
  }

  virtual void addArc(const DL_ArcData& d)
  {
    DL_WriterA* w;
    DL_Attributes a;
    if (addCommon(d, w, a)) _dxf->writeArc(*w, d, a);
  }

  virtual void addCircle(const DL_CircleData& d)
  {
    DL_WriterA* w;
    DL_Attributes a;
    if (addCommon(d, w, a)) _dxf->writeCircle(*w, d, a);
  }

#if 0 // TODO
  virtual void addBlock(const DL_BlockData& d)
  {
    DL_Attributes a = getAttributes();
    DL_WriterA* const w = find_part_writer(a.getLayer());
    if (w == NULL) return ;
    _dxf->writeBlock(*w, d, a);
  }

  virtual void endBlock()
  {
  }
#endif

#if 0 // not implemented
  virtual void addEllipse(const DL_EllipseData&) {}
  virtual void addPolyline(const DL_PolylineData&) {}
  virtual void addVertex(const DL_VertexData&) {}
  virtual void addSpline(const DL_SplineData&) {}
  virtual void addControlPoint(const DL_ControlPointData&) {}
  virtual void addKnot(const DL_KnotData&) {}
  virtual void addInsert(const DL_InsertData&) {}
  virtual void addMText(const DL_MTextData&) {}
  virtual void addMTextChunk(const char*) {}
  virtual void addText(const DL_TextData&) {}
  virtual void addDimAlign(const DL_DimensionData&, const DL_DimAlignedData&) {}
  virtual void addDimLinear(const DL_DimensionData&, const DL_DimLinearData&) {}
  virtual void addDimRadial(const DL_DimensionData&, const DL_DimRadialData&) {}
  virtual void addDimDiametric(const DL_DimensionData&, const DL_DimDiametricData&) {}
  virtual void addDimAngular(const DL_DimensionData&, const DL_DimAngularData&) {}
  virtual void addDimAngular3P(const DL_DimensionData&, const DL_DimAngular3PData&) {}
  virtual void addDimOrdinate(const DL_DimensionData&, const DL_DimOrdinateData&) {}
  virtual void addLeader(const DL_LeaderData&) {}
  virtual void addLeaderVertex(const DL_LeaderVertexData&) {}
  virtual void addHatch(const DL_HatchData&) {}
  virtual void addTrace(const DL_TraceData&) {}
  virtual void add3dFace(const DL_3dFaceData&) {}
  virtual void addSolid(const DL_SolidData&) {}
  virtual void addImage(const DL_ImageData&) {}
  virtual void linkImage(const DL_ImageDefData&) {}
  virtual void addHatchLoop(const DL_HatchLoopData&) {}
  virtual void addHatchEdge(const DL_HatchEdgeData&) {}
  virtual void endEntity() {}
  virtual void addComment(const char* comment) {}
  virtual void setVariableVector(const char*, double, double, double, int) {}
  virtual void setVariableString(const char*, const char*, int) {}
  virtual void setVariableInt(const char*, int, int) {}
  virtual void setVariableDouble(const char*, double, int) {}
  virtual void endSequence() {}
#endif // not implemented
};


int main(int ac, char** av)
{
  if (ac == 1) return -1;

  DL_Dxf dxf;

  layerFilter filter(&dxf);
  if (filter.has_failed())
  {
    ::printf("filter.has_failed()\n");
    return -1;
  }

  if (dxf.in(av[1], &filter) == false)
  {
    ::printf("dxf.in() == false\n");
    return -1;
  }

  return 0;
}
