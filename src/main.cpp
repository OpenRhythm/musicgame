#include "game.hpp"
#include "library/node.hpp"
int main()
{
    // Uncomment to list your own FretsOnFire folder
/*    node root_node("/home/Data/FoF/songs");
    directory root_dir(root_node);
    root_dir.list_dir();
*/
    GameManager game;
    game.start();
    return 0;
}
