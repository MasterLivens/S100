<?xml version="1.0" encoding="UTF-8"?>
<!-- Synthetic S-124-shaped fixture for IHO-S124-OPEN-LIST. Not official IHO schema-valid. -->
<S124:Dataset xmlns:S124="http://www.iho.int/S124/2.0"
              xmlns:gml="http://www.opengis.net/gml/3.2"
              xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
              productId="S-124">
  <S124:NavigationalWarning gml:id="NW.1">
    <S124:title>Synthetic Hazard Alpha</S124:title>
    <S124:message>Point hazard for CI</S124:message>
    <S124:timeISO>2024-01-01T00:00:00Z</S124:timeISO>
    <S124:geometry>
      <gml:Point gml:id="NW.1.pt" srsName="EPSG:4326">
        <gml:pos>0.2 0.2</gml:pos>
      </gml:Point>
    </S124:geometry>
  </S124:NavigationalWarning>
  <S124:NavigationalWarning gml:id="NW.2">
    <S124:title>Synthetic Area Bravo</S124:title>
    <S124:message>Area hazard for CI</S124:message>
    <S124:timeISO>2024-01-01T06:00:00Z</S124:timeISO>
    <S124:geometry>
      <gml:Polygon gml:id="NW.2.poly" srsName="EPSG:4326">
        <gml:exterior>
          <gml:LinearRing>
            <!-- lon lat; exact ring for tests -->
            <gml:posList>0.0 0.0 0.4 0.0 0.4 0.4 0.0 0.4 0.0 0.0</gml:posList>
          </gml:LinearRing>
        </gml:exterior>
      </gml:Polygon>
    </S124:geometry>
  </S124:NavigationalWarning>
</S124:Dataset>
